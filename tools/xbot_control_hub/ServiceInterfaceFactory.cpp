//
// Created by clemens on 4/30/24.
//

#include "ServiceInterfaceFactory.hpp"

#include <xbot/datatypes/ClaimPayload.hpp>
#include <xbot/datatypes/XbotHeader.hpp>

#include "endpoint_utils.hpp"
#include "spdlog/spdlog.h"
using namespace xbot::hub;

std::map<uint64_t, std::unique_ptr<ServiceState> > ServiceInterfaceFactory::endpoint_map_{};
std::shared_ptr<ServiceInterfaceFactory> ServiceInterfaceFactory::instance_ = nullptr;
std::recursive_mutex ServiceInterfaceFactory::state_mutex_{};
std::thread ServiceInterfaceFactory::io_thread_{};
Socket ServiceInterfaceFactory::io_socket_{"0.0.0.0"};
std::atomic_flag ServiceInterfaceFactory::stopped_{false};
std::chrono::time_point<std::chrono::steady_clock> ServiceInterfaceFactory::last_check_{std::chrono::seconds(0)};

bool ServiceInterfaceFactory::OnServiceDiscovered(std::string uid) {
    std::unique_lock lk{state_mutex_};

    uint32_t service_ip = 0;
    uint16_t service_port = 0;

    if (ServiceDiscovery::GetEndpoint(uid, service_ip, service_port) && service_ip != 0 && service_port != 0) {
        // Got valid enpdoint, create a state
        uint64_t key = BuildKey(service_ip, service_port);
        if (endpoint_map_.contains(key)) {
            spdlog::warn(
                "Service state already exists, overwriting with new state. This might have unforseen consequences.");
            endpoint_map_.erase(key);
        }
        endpoint_map_.emplace(key, std::make_unique<ServiceState>());
    }

    return true;
}

bool ServiceInterfaceFactory::OnEndpointChanged(std::string uid, uint32_t old_ip, uint16_t old_port, uint32_t new_ip,
                                                uint16_t new_port) {
    if (old_ip == 0 || old_port == 0) {
        // Invalid endpoint, we never stored this reference
        return true;
    }

    std::unique_lock lk{state_mutex_};

    // Got valid enpdoint, create a state
    uint64_t old_key = BuildKey(old_ip, old_port);
    uint64_t new_key = BuildKey(new_ip, new_port);

    if (endpoint_map_.contains(new_key)) {
        spdlog::warn(
            "Service state already exists, overwriting with new state. This might have unforseen consequences.");
        endpoint_map_.erase(new_key);
    }

    if (!endpoint_map_.contains(old_key)) {
        spdlog::warn("Service state did not exist, so we cannot update it. Creating a new one instead.");
        endpoint_map_.emplace(new_key, std::make_unique<ServiceState>());
    } else {
        endpoint_map_.emplace(new_key, std::move(endpoint_map_.at(old_key)));
        endpoint_map_.erase(old_key);
    }


    return true;
}

std::shared_ptr<ServiceInterfaceFactory> ServiceInterfaceFactory::GetInstance() {
    std::unique_lock lk{state_mutex_};
    if (instance_ == nullptr) {
        instance_ = std::make_shared<ServiceInterfaceFactory>();
    }
    return instance_;
}

bool ServiceInterfaceFactory::Start() {
    stopped_.clear();
    io_thread_ = std::thread{&ServiceInterfaceFactory::RunIo};

    return true;
}

void ServiceInterfaceFactory::RunIo() {
    // Set timeout so that we can detect missing heartbeat
    io_socket_.SetReceiveTimeoutMicros(config::default_heartbeat_micros);
    io_socket_.Start();
    uint32_t sender_ip;
    uint16_t sender_port;
    std::vector<uint8_t> packet{};
    while (!stopped_.test()) {
        if (std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::steady_clock::now() - last_check_) > std::chrono::microseconds(1000000)) {
            spdlog::debug("running checks");
            std::unique_lock lk{state_mutex_};
            // Claim all unclaimed services and check for timeouts.
            for (auto it = endpoint_map_.begin(); it != endpoint_map_.end();/* no increment */) {
                if (!it->second->claimed_successfully_) {
                    ClaimService(it->first);
                    ++it;
                } else {
                    // Check for timeout
                    if (std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::steady_clock::now() - it->second->last_heartbeat_received_) >
                        std::chrono::microseconds(
                            config::default_heartbeat_micros + config::heartbeat_jitter)) {
                        spdlog::warn("Service timed out, removing service.");
                        // Erase and advance iterator
                        it = endpoint_map_.erase(it);
                    } else {
                        // No timeout, go to next
                        ++it;
                    }
                }
            }
        }

        if (io_socket_.ReceivePacket(sender_ip, sender_port, packet)) {
            if (packet.size() >= sizeof(comms::datatypes::XbotHeader)) {
                const auto header = reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());

                if (header->payload_size != packet.size() - sizeof(comms::datatypes::XbotHeader)) {
                    spdlog::error("Got packet with invalid size");
                    continue;
                }
                uint64_t key = BuildKey(sender_ip, sender_port);

                if (header->message_type == comms::datatypes::MessageType::CLAIM) {
                    if (header->arg1 != 1) {
                        // Not actually an ack
                        spdlog::warn("received claim ack without arg1==1");
                        continue;
                    } {
                        std::unique_lock lk{state_mutex_};
                        if (!endpoint_map_.contains(key)) {
                            spdlog::warn("received claim ack from wrong service");
                            continue;
                        }
                        endpoint_map_.at(key)->claimed_successfully_ = true;
                        // Also count the ack as heartbeat in order to not instantly timeout
                        endpoint_map_.at(key)->last_heartbeat_received_ = std::chrono::steady_clock::now();;
                        spdlog::info("Successfully claimed service");
                    }
                } else if (header->message_type == comms::datatypes::MessageType::HEARTBEAT) {
                    {
                        std::unique_lock lk{state_mutex_};
                        if (!endpoint_map_.contains(key)) {
                            spdlog::warn("received heartbeat from wrong service");
                            continue;
                        }
                        endpoint_map_.at(key)->last_heartbeat_received_ = std::chrono::steady_clock::now();;
                    }
                } else if (header->message_type == comms::datatypes::MessageType::DATA) {
                    {
                        std::unique_lock lk{state_mutex_};
                        if (!endpoint_map_.contains(key)) {
                            spdlog::warn("got data from wrong service");
                            continue;
                        }
                        if (!endpoint_map_.at(key)->claimed_successfully_) {
                            spdlog::warn("Got data from an unclaimed service, dropping it.");
                            continue;
                        }
                    }
                    const auto payload_ptr = reinterpret_cast<void *>(
                        packet.data() + sizeof(comms::datatypes::XbotHeader));
                    spdlog::info("Got data!");
                }
            }
        }
    }
}

void ServiceInterfaceFactory::ClaimService(uint64_t key) {
    state_mutex_.lock();
    if (!endpoint_map_.contains(key)) {
        // Cannot try to claim a service which was not even discovered
        spdlog::error("Tried to claim a service which was not in the endpoint map");
        state_mutex_.unlock();
        return;
    }
    const auto &state = endpoint_map_.at(key);
    // Can unlock here, because even if the endpoint changes, the state remains at the same location
    state_mutex_.unlock();

    // Check, if we recently sent the claim. If not, try again
    auto now = std::chrono::steady_clock::now();

    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(now - state->last_claim_sent_);
    if (diff < std::chrono::microseconds(1000)) {
        return;
    }

    // Set it here, in case of error we also don't want to retry too often
    state->last_claim_sent_ = now;
    state->claimed_successfully_ = false;

    std::string my_ip{};
    uint16_t my_port;
    if (!io_socket_.GetEndpoint(my_ip, my_port) || my_ip == "0.0.0.0" || my_ip.empty() || my_port == 0) {
        spdlog::warn("Could not claim service, interface socket is not bound yet");
        return;
    }


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

    TransmitPacket(key, packet);
}

bool ServiceInterfaceFactory::TransmitPacket(uint64_t key, const std::vector<uint8_t> &data) {
    uint32_t service_ip = key >> 32;
    uint32_t service_port = key & 0xFFFF;

    if (service_ip == 0 || service_port == 0) {
        return false;
    }
    return io_socket_.TransmitPacket(service_ip, service_port, data);
}
