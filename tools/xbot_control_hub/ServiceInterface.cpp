//
// Created by clemens on 4/30/24.
//

#include "ServiceInterface.hpp"
#include <spdlog/spdlog.h>

#include <utility>
#include <xbot/datatypes/XbotHeader.hpp>

#include "endpoint_utils.hpp"
#include "ServiceDiscovery.hpp"
#include "ServiceInfo.hpp"

xbot::hub::ServiceInterface::ServiceInterface(std::string uid) : uid_(std::move(uid)), socket_("0.0.0.0") {
}

bool xbot::hub::ServiceInterface::OnServiceEndpointChanged() {
    return true;
}

bool xbot::hub::ServiceInterface::Start() {
    if(!UpdateHash()) {
        spdlog::error("Error starting service interface: Error calculating the hash. (ID: {})", uid_);
        return false;
    }

    stopped_.clear();
    io_thread_ = std::thread{&ServiceInterface::RunIo, this};

    return true;
}

void xbot::hub::ServiceInterface::SendClaim() {
    // Check, if we recently sent the claim. If not, try again
    auto now = std::chrono::steady_clock::now();

    if(std::chrono::duration_cast<std::chrono::milliseconds>(now - last_claim_sent_).count() < 1000) {
        return;
    }

    // Set it here, in case of error we also don't want to retry too often
    last_claim_sent_ = now;

    std::string ip{};
    uint16_t port;
    if(!socket_.GetEndpoint(ip, port) || ip == "0.0.0.0" || ip.empty() || port == 0) {
        spdlog::warn("Could not claim service, interface socket is not bound yet");
        return;
    }

    spdlog::info("Sending claim to service (ID: {})", uid_);

    std::vector<uint8_t> packet{};
    packet.resize(sizeof(comms::datatypes::XbotHeader)+sizeof(uint32_t)+sizeof(uint16_t));
    auto header = reinterpret_cast<comms::datatypes::XbotHeader*>(packet.data());
    header->message_type = comms::datatypes::MessageType::CLAIM;
    header->protocol_version = 1;
    header->arg1 = 0;
    header->arg2 = 0;
    header->sequence_no = 0;
    header->flags = 0;
    header->timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    header->payload_size = sizeof(uint32_t)+sizeof(uint16_t);
    auto ip_ptr = reinterpret_cast<uint32_t*>(packet.data() + sizeof(comms::datatypes::XbotHeader));
    auto port_ptr = reinterpret_cast<uint16_t*>(packet.data() + sizeof(comms::datatypes::XbotHeader)+sizeof(uint32_t));
    *ip_ptr = IpStringToInt(ip);
    *port_ptr = port;

    TransmitPacket(packet);

}

bool xbot::hub::ServiceInterface::TransmitPacket(const std::vector<uint8_t> &data) {
    uint32_t service_ip = 0;
    uint16_t service_port = 0;
    ServiceDiscovery::GetEndpoint(uid_, service_ip, service_port);
    if(service_ip == 0 || service_port == 0) {
        return false;
    }
    return socket_.TransmitPacket(service_ip, service_port, data);
}

void xbot::hub::ServiceInterface::RunIo() {
    socket_.Start();
    uint32_t sender_ip;
    uint16_t sender_port;
    std::vector<uint8_t> packet{};
    while(!stopped_.test()) {
        if(!claimed_successfully_) {
            SendClaim();
        }

        if(socket_.ReceivePacket(sender_ip, sender_port, packet)) {
            if (packet.size() >= sizeof(comms::datatypes::XbotHeader)) {
                const auto header = reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());



                if(header->message_type == comms::datatypes::MessageType::CLAIM) {

                }
            }
        }
    }
}

bool xbot::hub::ServiceInterface::UpdateHash() {
    const auto service_info = ServiceDiscovery::GetServiceInfo(uid_);

    if(service_info == nullptr) {
        return false;
    }

    service_info_hash_ = 0;
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[0]);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[1]);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[2]);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[3]);
    service_info_hash_ ^= std::hash<uint16_t>{}(service_info->service_id_);
    service_info_hash_ ^= std::hash<std::string>{}(service_info->type_);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->version_);

    return true;
}

size_t xbot::hub::ServiceInterface::Hash(xbot::comms::datatypes::XbotHeader &header) {
    service_info_hash_ = 0;
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[0]);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[1]);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[2]);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->node_id_[3]);
    service_info_hash_ ^= std::hash<uint16_t>{}(service_info->service_id_);
    service_info_hash_ ^= std::hash<std::string>{}(service_info->type_);
    service_info_hash_ ^= std::hash<uint32_t>{}(service_info->version_);
}
