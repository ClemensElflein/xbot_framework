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
    ip = service_info.ip_;
    port = service_info.port_;
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

void ServiceDiscovery::Run() {
  std::vector<uint8_t> packet{};
  uint32_t sender_ip;
  uint16_t sender_port;
  while (!stopped_.test()) {
    if (sd_socket_.ReceivePacket(sender_ip, sender_port, packet)) {
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

            // Convert endpoint string to int
            uint32_t ip =
                IpStringToInt(json.at("endpoint").at("ip").get<std::string>());

            std::vector<ServiceInputInfo> inputs{};

            uint32_t node_id[] = {json.at("nid").at(0).get<uint32_t>(),
                                  json.at("nid").at(1).get<uint32_t>(),
                                  json.at("nid").at(2).get<uint32_t>(),
                                  json.at("nid").at(3).get<uint32_t>()};

            // Build the ServiceInfo object from the received data.
            ServiceInfo info{node_id,
                             json.at("sid").get<uint16_t>(),
                             ip,
                             json.at("endpoint").at("port").get<uint16_t>(),
                             json.at("desc").at("type").get<std::string>(),
                             json.at("desc").at("version").get<uint32_t>(),
                             inputs};

            // Check for valid endpoint, if none is given the service is not
            // reachable, so we drop it
            if (info.ip_ == 0 || info.port_ == 0) {
              spdlog::warn(
                  "Service registered with invalid endpoint. Ignoring. (ID: "
                  "{}, endpoint: {})",
                  info.unique_id_, EndpointIntToString(info.ip_, info.port_));
              continue;
            }

            // Scope for locking the discovered_services_ map
            {
              std::unique_lock lk(sd_mutex_);
              if (discovered_services_.contains(info.unique_id_)) {
                // Check, if service endpoint was updated (every thing else is
                // constant) and update
                if (auto &old_service_info =
                        discovered_services_.at(info.unique_id_);
                    old_service_info.ip_ != info.ip_ ||
                    old_service_info.port_ != info.port_) {
                  spdlog::info("Endpoint updated (ID: {}, new endpoint: {})",
                               info.unique_id_,
                               EndpointIntToString(info.ip_, info.port_));
                  // Backup the old infos, so that we can pass them to the
                  // callback
                  const auto old_ip = old_service_info.ip_;
                  const auto old_port = old_service_info.port_;

                  // Update the entry
                  old_service_info.ip_ = info.ip_;
                  old_service_info.port_ = info.port_;

                  // Notify callbacks
                  for (const auto &callback : registered_callbacks_) {
                    callback->OnEndpointChanged(info.unique_id_, old_ip,
                                                old_port, info.ip_, info.port_);
                  }
                }
              } else {
                spdlog::info("Found new service (ID: {}, endpoint: {})",
                             info.unique_id_,
                             EndpointIntToString(info.ip_, info.port_));
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
