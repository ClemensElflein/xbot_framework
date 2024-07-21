//
// Created by clemens on 7/17/24.
//

#ifndef SERVICEINTERFACEBASE_HPP
#define SERVICEINTERFACEBASE_HPP
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "Plugin.hpp"
#include "ServiceIO.hpp"

namespace xbot::serviceif {
class ServiceInterfaceBase : public xbot::serviceif::ServiceIOCallbacks,
                             public xbot::serviceif::ServiceDiscoveryCallbacks {
 public:
  ServiceInterfaceBase(uint16_t service_id, std::string type, uint32_t version,
                       Context ctx);

  void Start();

 protected:
  const uint16_t service_id_;
  // Type of the service (e.g. IMU Service)
  const std::string type_;
  // Version of the service (changes whenever the interface introduces breaking
  // changes)
  const uint32_t version_;

  // uid of the bound service
  std::string uid_{};

  bool StartTransaction(bool is_configuration = false);

  bool CommitTransaction();

  bool SendData(uint16_t target_id, const void *data, size_t size,
                bool is_configuration);

 public:
  bool OnServiceDiscovered(std::string uid) final;
  bool OnEndpointChanged(std::string uid, uint32_t old_ip, uint16_t old_port,
                         uint32_t new_ip, uint16_t new_port) final;

 private:
  void FillHeader();

  // Scratch space for the header.
  xbot::datatypes::XbotHeader header_{};
  std::vector<uint8_t> buffer_{};
  bool transaction_started_{false};
  bool is_configuration_transaction_{false};

  std::recursive_mutex state_mutex_{};

  Context ctx{};
};
}  // namespace xbot::serviceif

#endif  // SERVICEINTERFACEBASE_HPP
