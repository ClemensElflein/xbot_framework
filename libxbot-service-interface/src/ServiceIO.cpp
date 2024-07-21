//
// Created by clemens on 4/30/24.
//

#include <spdlog/spdlog.h>

#include <map>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>
#include <xbot-service-interface/ServiceIO.hpp>
#include <xbot-service-interface/Socket.hpp>
#include <xbot/datatypes/ClaimPayload.hpp>

#include "xbot-service-interface/endpoint_utils.hpp"

using namespace xbot::serviceif;

/**
 * Maps endpoint to state. Pointers are used so that we can move the state for
 * cheap once the endpoint changes.
 */
std::map<uint64_t, std::unique_ptr<ServiceState>> endpoint_map_{};
std::map<std::string, uint64_t> inverse_endpoint_map_{};

// Protects inverse_endpoint_map_, endpoint_map_ and instance_
std::recursive_mutex state_mutex_{};
ServiceIO *instance_ = nullptr;

std::thread io_thread_{};
Socket io_socket_{"0.0.0.0"};
std::atomic_flag stopped_{false};
// track when we last checked for claims and timeouts
std::chrono::time_point<std::chrono::steady_clock> last_check_{
    std::chrono::seconds(0)};
;

// keep a list of callbacks for each service
std::map<std::string, std::vector<ServiceIOCallbacks *>>
    registered_callbacks_{};

bool ServiceIO::OnServiceDiscovered(std::string uid) {
  std::unique_lock lk{state_mutex_};

  uint32_t service_ip = 0;
  uint16_t service_port = 0;

  if (ServiceDiscovery::GetEndpoint(uid, service_ip, service_port) &&
      service_ip != 0 && service_port != 0) {
    // Got valid enpdoint, create a state
    uint64_t key = BuildKey(service_ip, service_port);
    if (endpoint_map_.contains(key)) {
      spdlog::warn(
          "Service state already exists, overwriting with new state. This "
          "might have unforseen consequences.");
      endpoint_map_.erase(key);
    }
    std::unique_ptr<ServiceState> state = std::make_unique<ServiceState>();
    state->uid = uid;
    endpoint_map_.emplace(key, std::move(state));
    inverse_endpoint_map_[uid] = key;
  }

  return true;
}

bool ServiceIO::OnEndpointChanged(std::string uid, uint32_t old_ip,
                                  uint16_t old_port, uint32_t new_ip,
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
        "Service state already exists, overwriting with new state. This might "
        "have unforseen consequences.");
    endpoint_map_.erase(new_key);
  }

  if (!endpoint_map_.contains(old_key)) {
    spdlog::warn(
        "Service state did not exist, so we cannot update it. Creating a new "
        "one instead.");
    endpoint_map_.emplace(new_key, std::make_unique<ServiceState>());
  } else {
    endpoint_map_.emplace(new_key, std::move(endpoint_map_.at(old_key)));
    endpoint_map_.erase(old_key);
  }

  inverse_endpoint_map_[uid] = new_key;

  return true;
}

ServiceIO *ServiceIO::GetInstance() {
  std::unique_lock lk{state_mutex_};
  if (instance_ == nullptr) {
    instance_ = new ServiceIO();
  }
  return instance_;
}

bool ServiceIO::Start() {
  stopped_.clear();
  io_thread_ = std::thread{&ServiceIO::RunIo};

  return true;
}
void ServiceIO::RegisterCallbacks(const std::string &uid,
                                  ServiceIOCallbacks *callbacks) {
  std::unique_lock lk{state_mutex_};
  auto &vector = registered_callbacks_[uid];

  // Check, if callbacks already registered
  if (std::find(vector.begin(), vector.end(), callbacks) != vector.end()) {
    return;
  }

  // add the callbacks
  vector.push_back(callbacks);
}
void ServiceIO::UnregisterCallbacks(ServiceIOCallbacks *callbacks) {
  std::unique_lock lk{state_mutex_};
  for (auto [service_id, callback_list] : registered_callbacks_) {
    for (auto cb_it = callback_list.begin(); cb_it != callback_list.end();) {
      if (*cb_it == callbacks) {
        // Erase
        cb_it = callback_list.erase(cb_it);
      } else {
        // Skip
        ++cb_it;
      }
    }
  }
}

bool ServiceIO::SendData(const std::string &uid,
                         const std::vector<uint8_t> &data) {
  uint64_t endpoint;
  {
    std::unique_lock lk{state_mutex_};
    if (!inverse_endpoint_map_.contains(uid)) {
      spdlog::warn("no endpoint for service {}", uid);
      return false;
    }
    endpoint = inverse_endpoint_map_.at(uid);
  }

  return TransmitPacket(endpoint, data);
}

void ServiceIO::RunIo() {
  // Set timeout so that we can detect missing heartbeat
  io_socket_.SetReceiveTimeoutMicros(config::default_heartbeat_micros);
  io_socket_.Start();
  uint32_t sender_ip;
  uint16_t sender_port;
  std::vector<uint8_t> packet{};
  while (!stopped_.test()) {
    if (std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - last_check_) >
        std::chrono::microseconds(1000000)) {
      spdlog::debug("running checks");
      std::unique_lock lk{state_mutex_};
      // Claim all unclaimed services and check for timeouts.
      for (auto it = endpoint_map_.begin(); it != endpoint_map_.end();
           /* no increment */) {
        if (!it->second->claimed_successfully_) {
          ClaimService(it->first);
          ++it;
        } else {
          // Check for timeout
          if (std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::steady_clock::now() -
                  it->second->last_heartbeat_received_) >
              std::chrono::microseconds(config::default_heartbeat_micros +
                                        config::heartbeat_jitter)) {
            spdlog::warn("Service timed out, removing service.");

            const auto &uid = it->second->uid;

            // Drop service from discovery, so that it will get
            // rediscovered later
            ServiceDiscovery::DropService(uid);

            // Notify callbacks for that service
            if (const auto cb_it = registered_callbacks_.find(uid);
                cb_it != registered_callbacks_.end()) {
              for (const auto &cb : cb_it->second) {
                cb->OnServiceDisconnected(uid);
              }
            }

            // Erase and advance iterator
            inverse_endpoint_map_.erase(uid);
            it = endpoint_map_.erase(it);
          } else {
            // No timeout, go to next
            ++it;
          }
        }
      }
    }

    if (io_socket_.ReceivePacket(sender_ip, sender_port, packet)) {
      if (packet.size() >= sizeof(datatypes::XbotHeader)) {
        const auto header =
            reinterpret_cast<datatypes::XbotHeader *>(packet.data());
        if (header->payload_size !=
            packet.size() - sizeof(datatypes::XbotHeader)) {
          spdlog::error("Got packet with invalid size");
          continue;
        }
        uint64_t key = BuildKey(sender_ip, sender_port);

        const uint8_t *const payload_buffer =
            packet.data() + sizeof(datatypes::XbotHeader);

        switch (header->message_type) {
          case datatypes::MessageType::CLAIM:
            HandleClaimMessage(key, header, payload_buffer,
                               header->payload_size);
            break;
          case datatypes::MessageType::DATA:
            HandleDataMessage(key, header, payload_buffer,
                              header->payload_size);
            break;
          case datatypes::MessageType::CONFIGURATION_REQUEST:
            HandleConfigurationRequest(key, header, payload_buffer,
                                       header->payload_size);
            break;
          case datatypes::MessageType::HEARTBEAT:
            HandleHeartbeatMessage(key, header, payload_buffer,
                                   header->payload_size);
            break;
          case datatypes::MessageType::TRANSACTION:
            if (header->arg1 == 0) {
              HandleDataTransaction(key, header, payload_buffer,
                                    header->payload_size);
            } else {
              spdlog::warn("Got transaction with unknown type");
            }
            break;
          default:
            spdlog::warn("Got message of unknown type");
            break;
        }
      }
    }
  }
}

void ServiceIO::ClaimService(uint64_t key) {
  std::unique_lock lk{state_mutex_};
  if (!endpoint_map_.contains(key)) {
    // Cannot try to claim a service which was not even discovered
    spdlog::error("Tried to claim a service which was not in the endpoint map");
    return;
  }
  const auto &state = endpoint_map_.at(key);

  // Check, if we recently sent the claim. If not, try again
  auto now = std::chrono::steady_clock::now();

  auto diff = std::chrono::duration_cast<std::chrono::microseconds>(
      now - state->last_claim_sent_);
  if (diff < std::chrono::microseconds(1000)) {
    return;
  }

  // Set it here, in case of error we also don't want to retry too often
  state->last_claim_sent_ = now;
  state->claimed_successfully_ = false;

  std::string my_ip{};
  uint16_t my_port;
  if (!io_socket_.GetEndpoint(my_ip, my_port) || my_ip == "0.0.0.0" ||
      my_ip.empty() || my_port == 0) {
    spdlog::warn("Could not claim service, interface socket is not bound yet");
    return;
  }

  std::vector<uint8_t> packet{};
  packet.resize(sizeof(datatypes::XbotHeader) +
                sizeof(datatypes::ClaimPayload));
  auto header = reinterpret_cast<datatypes::XbotHeader *>(packet.data());
  header->message_type = datatypes::MessageType::CLAIM;
  header->protocol_version = 1;
  header->arg1 = 0;
  header->arg2 = 0;
  header->sequence_no = 0;
  header->flags = 0;
  header->timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                          std::chrono::steady_clock::now().time_since_epoch())
                          .count();
  header->payload_size = sizeof(datatypes::ClaimPayload);
  auto payload_ptr = reinterpret_cast<datatypes::ClaimPayload *>(
      packet.data() + sizeof(datatypes::XbotHeader));
  payload_ptr->target_ip = IpStringToInt(my_ip);
  payload_ptr->target_port = my_port;
  payload_ptr->heartbeat_micros = config::default_heartbeat_micros;
  spdlog::info("Sending Service Claim");
  TransmitPacket(key, packet);
}

bool ServiceIO::TransmitPacket(uint64_t key, const std::vector<uint8_t> &data) {
  uint32_t service_ip = key >> 32;
  uint32_t service_port = key & 0xFFFF;

  if (service_ip == 0 || service_port == 0) {
    return false;
  }
  return io_socket_.TransmitPacket(service_ip, service_port, data);
}
void ServiceIO::HandleClaimMessage(uint64_t key,
                                   xbot::datatypes::XbotHeader *header,
                                   const uint8_t *payload, size_t payload_len) {
  if (header->arg1 != 1) {
    // Not actually an ack
    spdlog::warn("received claim ack without arg1==1");
    return;
  }

  std::unique_lock lk{state_mutex_};
  if (!endpoint_map_.contains(key)) {
    spdlog::warn("received claim ack from wrong service");
    return;
  }
  const auto &ptr = endpoint_map_.at(key);
  // Also count the ack as heartbeat in order to not instantly timeout
  ptr->last_heartbeat_received_ = std::chrono::steady_clock::now();

  if (ptr->claimed_successfully_) {
    spdlog::warn("claim ack from already claimed service");
    return;
  }
  ptr->claimed_successfully_ = true;
  spdlog::info("Successfully claimed service");

  // Notify callbacks for that service
  if (const auto it = registered_callbacks_.find(ptr->uid);
      it != registered_callbacks_.end()) {
    for (const auto &cb : it->second) {
      cb->OnServiceConnected(ptr->uid);
    }
  }
}
void ServiceIO::HandleDataMessage(uint64_t key,
                                  xbot::datatypes::XbotHeader *header,
                                  const uint8_t *payload, size_t payload_len) {
  {
    std::unique_lock lk{state_mutex_};
    if (!endpoint_map_.contains(key)) {
      spdlog::debug("got data from wrong service");
      return;
    }
    if (!endpoint_map_.at(key)->claimed_successfully_) {
      spdlog::debug("Got data from an unclaimed service, dropping it.");
      return;
    }
  }
  const auto &ptr = endpoint_map_.at(key);

  // Notify callbacks for that service
  if (const auto it = registered_callbacks_.find(ptr->uid);
      it != registered_callbacks_.end()) {
    for (const auto &cb : it->second) {
      cb->OnData(ptr->uid, header->timestamp, header->arg2, payload,
                 header->payload_size);
    }
  }
}
void ServiceIO::HandleDataTransaction(uint64_t key,
                                      xbot::datatypes::XbotHeader *header,
                                      const uint8_t *payload,
                                      size_t payload_len) {
  {
    std::unique_lock lk{state_mutex_};
    if (!endpoint_map_.contains(key)) {
      // This happens if we restart the interface and an unknown service sends
      // us data.
      spdlog::debug("got data from wrong service");
      return;
    }
    if (!endpoint_map_.at(key)->claimed_successfully_) {
      // This happens if we restart the interface and a previously claimed
      // service is still sending data.
      spdlog::debug("Got data from an unclaimed service, dropping it.");
      return;
    }
  }
  const auto &state_ptr = endpoint_map_.at(key);

  // Notify callbacks for that service
  if (const auto it = registered_callbacks_.find(state_ptr->uid);
      it != registered_callbacks_.end()) {
    for (const auto &cb : it->second) {
      cb->OnTransactionStart(header->timestamp);
      // Go through all data packets in the transaction
      size_t processed_len = 0;
      while (processed_len + sizeof(datatypes::DataDescriptor) <=
             header->payload_size) {
        // we have at least enough data for the next descriptor, read it
        const auto descriptor =
            reinterpret_cast<const datatypes::DataDescriptor *>(payload +
                                                                processed_len);
        size_t data_size = descriptor->payload_size;
        if (processed_len + sizeof(datatypes::DataDescriptor) + data_size <=
            header->payload_size) {
          // we can safely read the data
          cb->OnData(
              state_ptr->uid, header->timestamp, descriptor->target_id,
              payload + processed_len + sizeof(datatypes::DataDescriptor),
              data_size);
        } else {
          spdlog::error(
              "Error parsing transaction, header payload size does not "
              "match transaction size!");
          break;
        }
        processed_len += data_size + sizeof(datatypes::DataDescriptor);
      }

      if (processed_len != header->payload_size) {
        spdlog::warn("Transaction size mismatch!");
      }

      cb->OnTransactionEnd();
    }
  }
}
void ServiceIO::HandleHeartbeatMessage(uint64_t key,
                                       xbot::datatypes::XbotHeader *header,
                                       const uint8_t *payload,
                                       size_t payload_len) {
  std::unique_lock lk{state_mutex_};
  if (!endpoint_map_.contains(key)) {
    spdlog::warn("received heartbeat from wrong service");
    return;
  }
  endpoint_map_.at(key)->last_heartbeat_received_ =
      std::chrono::steady_clock::now();
}
void ServiceIO::HandleConfigurationRequest(uint64_t key,
                                           xbot::datatypes::XbotHeader *header,
                                           const uint8_t *payload,
                                           size_t payload_len) {
  std::unique_lock lk{state_mutex_};
  if (!endpoint_map_.contains(key)) {
    spdlog::debug("got config request from wrong service");
    return;
  }
  if (!endpoint_map_.at(key)->claimed_successfully_) {
    spdlog::debug("Got config request from an unclaimed service, dropping it.");
    return;
  }

  const auto &ptr = endpoint_map_.at(key);

  // Notify callbacks for that service
  bool configuration_handled = false;
  if (const auto it = registered_callbacks_.find(ptr->uid);
      it != registered_callbacks_.end()) {
    for (const auto &cb : it->second) {
      if (cb->OnConfigurationRequested(ptr->uid)) {
        configuration_handled = true;
        break;
      }
    }
  }
  if (!configuration_handled) {
    spdlog::warn(
        "service {} requires configuration, but no handler provided any "
        "configuration. "
        "The service won't start.",
        ptr->uid);
  }
}
