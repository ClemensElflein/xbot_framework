//
// Created by clemens on 4/29/24.
//

#include "ServiceDiscovery.hpp"

#include <mutex>
#include <nlohmann/json.hpp>
#include <xbot/config.hpp>
#include <xbot/datatypes/XbotHeader.hpp>

#include "endpoint_utils.hpp"
#include "spdlog/spdlog.h"

namespace xbot::hub {
Socket ServiceDiscovery::sd_socket_{"0.0.0.0", config::multicast_port};
std::thread ServiceDiscovery::sd_thread_{};
std::atomic_flag ServiceDiscovery::stopped_{false};
std::recursive_mutex ServiceDiscovery::sd_mutex_{};
std::map<std::string, ServiceInfo> ServiceDiscovery::discovered_services_{};
std::vector<std::shared_ptr<ServiceDiscoveryCallbacks>>
    ServiceDiscovery::registered_callbacks_{};

bool ServiceDiscovery::GetEndpoint(const std::string &uid, uint32_t &ip,
                                   uint16_t &port) {
  std::unique_lock lk(sd_mutex_);
  if (discovered_services_.contains(uid)) {
    auto &service_info = discovered_services_.at(uid);
    ip = service_info.ip;
    port = service_info.port;
    return true;
  }
  ip = 0;
  port = 0;
  return false;
}

bool ServiceDiscovery::Start() {
  if (!sd_socket_.Start()) return false;

  if (!sd_socket_.JoinMulticast(config::sd_multicast_address)) return false;

  // Start the ServiceDiscovery Thread
  stopped_.clear();
  sd_thread_ = std::thread{Run};
  return true;
}

void ServiceDiscovery::RegisterCallbacks(
    std::shared_ptr<ServiceDiscoveryCallbacks> callbacks) {
  if (callbacks == nullptr) {
    return;
  }
  std::unique_lock lk(sd_mutex_);
  registered_callbacks_.push_back(callbacks);
}

std::unique_ptr<ServiceInfo> ServiceDiscovery::GetServiceInfo(
    const std::string &uid) {
  std::unique_lock lk(sd_mutex_);
  if (!discovered_services_.contains(uid)) {
    return nullptr;
  }

  return std::make_unique<ServiceInfo>(discovered_services_.at(uid));
}
std::unique_ptr<std::map<std::string, ServiceInfo>>
ServiceDiscovery::GetAllSerivces() {
  std::unique_lock lk(sd_mutex_);
  return std::make_unique<std::map<std::string, ServiceInfo>>(
      discovered_services_);
}

void ServiceDiscovery::Run() {
  std::vector<uint8_t> packet{};
  uint32_t sender_ip;
  uint16_t sender_port;
  // While not stopped
  while (!stopped_.test()) {
    // Try receive a packet, this will return false on timeout.
    if (sd_socket_.ReceivePacket(sender_ip, sender_port, packet)) {
      // Check, if packet has at least enough space for our header
      if (packet.size() >= sizeof(comms::datatypes::XbotHeader)) {
        const auto header =
            reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());

        if (header->message_type !=
            comms::datatypes::MessageType::SERVICE_ADVERTISEMENT) {
          spdlog::warn(
              "Service Discovery socket got non-service discovery message");
          continue;
        }

        // Validate reported length
        if (packet.size() ==
            header->payload_size + sizeof(comms::datatypes::XbotHeader)) {
          try {
            const auto json = nlohmann::json::from_cbor(
                packet.begin() + sizeof(comms::datatypes::XbotHeader),
                packet.end());

            // Build the ServiceInfo object from the received data.
            ServiceInfo info = json;

            // Check for valid endpoint, if none is given the service is not
            // reachable, so we drop it
            if (info.ip == 0 || info.port == 0) {
              spdlog::warn(
                  "Service registered with invalid endpoint. Ignoring. (ID: "
                  "{}, endpoint: {})",
                  info.unique_id_, EndpointIntToString(info.ip, info.port));
              continue;
            }

            // Scope for locking the discovered_services_ map
            {
              std::unique_lock lk(sd_mutex_);
              if (discovered_services_.contains(info.unique_id_)) {
                // Check, if service endpoint was updated
                // (every thing else is constant) and update
                if (auto &old_service_info =
                        discovered_services_.at(info.unique_id_);
                    old_service_info.ip != info.ip ||
                    old_service_info.port != info.port) {
                  spdlog::info("Endpoint updated (ID: {}, new endpoint: {})",
                               info.unique_id_,
                               EndpointIntToString(info.ip, info.port));
                  // Backup the old infos, so that we can pass them to the
                  // callback
                  const auto old_ip = old_service_info.ip;
                  const auto old_port = old_service_info.port;

                  // Update the entry
                  old_service_info.ip = info.ip;
                  old_service_info.port = info.port;

                  // Notify callbacks
                  for (const auto &callback : registered_callbacks_) {
                    callback->OnEndpointChanged(info.unique_id_, old_ip,
                                                old_port, info.ip, info.port);
                  }
                }
              } else {
                spdlog::info("Found new service (ID: {}, endpoint: {})",
                             info.unique_id_,
                             EndpointIntToString(info.ip, info.port));
                discovered_services_.emplace(info.unique_id_, info);
                // Notify callbacks
                for (const auto &callback : registered_callbacks_) {
                  callback->OnServiceDiscovered(info.unique_id_);
                }
              }
            }
          } catch (std::exception &e) {
            spdlog::error("Got exception during service discovery: {}",
                          e.what());
          }
        }
      }
    }
  }
}
}  // namespace xbot::hub
