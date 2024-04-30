//
// Created by clemens on 4/30/24.
//

#include "ServiceInterfaceBase.hpp"
#include <spdlog/spdlog.h>

#include <utility>
#include <xbot/datatypes/XbotHeader.hpp>
#include <xbot/datatypes/ClaimPayload.hpp>

#include "endpoint_utils.hpp"
#include "ServiceDiscovery.hpp"

xbot::hub::ServiceInterfaceBase::ServiceInterfaceBase(std::string uid) : uid_(std::move(uid)), socket_("0.0.0.0") {
}

bool xbot::hub::ServiceInterfaceBase::OnServiceEndpointChanged() {
    last_claim_sent_ = std::chrono::time_point<std::chrono::steady_clock>{std::chrono::microseconds(0)};
    claimed_successfully_.clear();
    return true;
}

bool xbot::hub::ServiceInterfaceBase::Start() {
    stopped_.clear();
    io_thread_ = std::thread{&ServiceInterfaceBase::RunIo, this};

    return true;
}


void xbot::hub::ServiceInterfaceBase::SendClaim() {
    // Check, if we recently sent the claim. If not, try again
    auto now = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - last_claim_sent_);
    if (diff < std::chrono::microseconds(1000)) {
        return;
    }

    // Set it here, in case of error we also don't want to retry too often
    last_claim_sent_ = now;
    claimed_successfully_.clear();

    std::string my_ip{};
    uint16_t my_port;
    if (!socket_.GetEndpoint(my_ip, my_port) || my_ip == "0.0.0.0" || my_ip.empty() || my_port == 0) {
        spdlog::warn("Could not claim service, interface socket is not bound yet");
        return;
    }

    ServiceDiscovery::GetEndpoint(uid_, service_ip_, service_port_);
    if (service_ip_ == 0 || service_port_ == 0) {
        return;
    }

    spdlog::info("Sending claim to service (ID: {})", uid_);

    std::vector<uint8_t> packet{};
    packet.resize(sizeof(comms::datatypes::XbotHeader) + sizeof(comms::datatypes::ClaimPayload));
    auto header = reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());
    header->message_type = comms::datatypes::MessageType::CLAIM;
    header->protocol_version = 1;
    header->arg1 = 0;
    header->arg2 = 0;
    header->sequence_no = 0;
    header->flags = 0;
    header->timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    header->payload_size = sizeof(comms::datatypes::ClaimPayload);
    auto payload_ptr = reinterpret_cast<comms::datatypes::ClaimPayload *>(
        packet.data() + sizeof(comms::datatypes::XbotHeader));
    payload_ptr->target_ip = IpStringToInt(my_ip);
    payload_ptr->target_port = my_port;
    payload_ptr->heartbeat_micros = config::default_heartbeat_micros;

    TransmitPacket(packet);
}

bool xbot::hub::ServiceInterfaceBase::TransmitPacket(const std::vector<uint8_t> &data) {
    if (service_ip_ == 0 || service_port_ == 0) {
        return false;
    }
    return socket_.TransmitPacket(service_ip_, service_port_, data);
}

void xbot::hub::ServiceInterfaceBase::RunIo() {
    claimed_successfully_.clear();
    service_ip_ = 0;
    service_port_ = 0;
    // Set timeout so that we can detect missing heartbeat
    socket_.SetReceiveTimeoutMicros(config::default_heartbeat_micros);
    socket_.Start();
    uint32_t sender_ip;
    uint16_t sender_port;
    std::vector<uint8_t> packet{};
    while (!stopped_.test()) {
        if (!claimed_successfully_.test()) {
            SendClaim();
        }

        if (socket_.ReceivePacket(sender_ip, sender_port, packet)) {
            if (packet.size() >= sizeof(comms::datatypes::XbotHeader)) {
                const auto header = reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());

                if(header->payload_size != packet.size() - sizeof(comms::datatypes::XbotHeader)) {
                    spdlog::error("Got packet with invalid size");
                    continue;
                }

                if (header->message_type == comms::datatypes::MessageType::CLAIM) {
                    if (sender_ip != service_ip_ || sender_port != service_port_) {
                        // Ack from wrong service, ignore.
                        spdlog::warn("received claim ack from wrong service");
                        continue;
                    }

                    if (header->arg1 != 1) {
                        // Not actually an ack
                        spdlog::warn("received claim ack without arg1==1");
                        continue;
                    }

                    claimed_successfully_.test_and_set();
                    spdlog::info("Successfully claimed service");

                    // Also count the ack as heartbeat in order to not instantly timeout
                    last_heartbeat_received_ = std::chrono::steady_clock::now();
                } else if (header->message_type == comms::datatypes::MessageType::HEARTBEAT) {
                    last_heartbeat_received_ = std::chrono::steady_clock::now();
                } else if(header->message_type == comms::datatypes::MessageType::DATA) {
                    if (!claimed_successfully_.test()) {
                        spdlog::warn("Got data from an unclaimed service, dropping it.");
                        continue;
                    }
                    const auto payload_ptr = reinterpret_cast<void *>(packet.data() + sizeof(comms::datatypes::XbotHeader));
                    OnOutputDataReceived(header->arg2, payload_ptr, header->payload_size);
                }
            }
        }

        // Check for heartbeat timeout, if timeout occurs reclaim the service
        if (std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - last_heartbeat_received_) > std::chrono::microseconds(
                config::default_heartbeat_micros + config::heartbeat_jitter)) {
            spdlog::warn("Service timed out");
            last_claim_sent_ = std::chrono::time_point<std::chrono::steady_clock>{std::chrono::microseconds(0)};
            claimed_successfully_.clear();
        }
    }
}
