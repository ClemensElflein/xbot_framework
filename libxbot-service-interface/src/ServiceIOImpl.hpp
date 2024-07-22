//
// Created by clemens on 7/21/24.
//

#ifndef XBOT_FRAMEWORK_SERVICEIOIMPL_HPP
#define XBOT_FRAMEWORK_SERVICEIOIMPL_HPP

#include <chrono>
#include <xbot-service-interface/ServiceDiscovery.hpp>
#include <xbot-service-interface/ServiceIO.hpp>

namespace xbot::serviceif {
// Keep track of the state of each service (claimed or not, timeout)
struct ServiceState {
  // track, if we have claimed the service successfully.
  // If the service is claimed it will send its outputs to this interface.
  bool claimed_successfully_{false};

  // track when we sent the last claim, so that we don't spam the service
  std::chrono::time_point<std::chrono::steady_clock> last_claim_sent_{
      std::chrono::seconds(0)};
  std::chrono::time_point<std::chrono::steady_clock> last_heartbeat_received_{
      std::chrono::seconds(0)};

  // keep the original service uid
  std::string uid{};
};

/**
 * ServiceIO subscribes to ServiceDiscovery and claims all services anyone
 * is interested in. It keeps track of the timeouts and redirects the actual
 * data to the actual subscribers.
 */
class ServiceIOImpl : public ServiceIO, public ServiceDiscoveryCallbacks {
 public:
  bool OnServiceDiscovered(std::string uid) override;

  bool OnEndpointChanged(std::string uid, uint32_t old_ip, uint16_t old_port,
                         uint32_t new_ip, uint16_t new_port) override;

  static ServiceIOImpl *GetInstance();

  bool Start();

  bool Stop();

  // Deleted copy constructor
  ServiceIOImpl(ServiceIOImpl &other) = delete;

  // Deleted assignment operator
  void operator=(const ServiceIO &) = delete;

  /**
   * Register callbacks for a specific uid
   * @param uid the UID to listen to
   * @param callbacks pointer to the callbacks
   */
  void RegisterCallbacks(const std::string &uid,
                         ServiceIOCallbacks *callbacks) override;

  /**
   * Unregister callbacks for all uids
   * @param callbacks callback pointer
   */
  void UnregisterCallbacks(ServiceIOCallbacks *callbacks) override;

  /**
   * Send data to a given service target
   */
  bool SendData(const std::string &uid,
                const std::vector<uint8_t> &data) override;

  explicit ServiceIOImpl(ServiceDiscoveryImpl *serviceDiscovery);
  ~ServiceIOImpl() override = default;

  bool OK() final;

 private:
  static inline uint64_t BuildKey(uint32_t ip, uint16_t port) {
    return static_cast<uint64_t>(ip) << 32 | port;
  }

  ServiceDiscoveryImpl *const service_discovery;

  void RunIo();

  void ClaimService(uint64_t key);

  bool TransmitPacket(uint64_t key, const std::vector<uint8_t> &data);

  void HandleClaimMessage(uint64_t key, datatypes::XbotHeader *header,
                          const uint8_t *payload, size_t payload_len);
  void HandleDataMessage(uint64_t key, datatypes::XbotHeader *header,
                         const uint8_t *payload, size_t payload_len);
  void HandleDataTransaction(uint64_t key, datatypes::XbotHeader *header,
                             const uint8_t *payload, size_t payload_len);
  void HandleHeartbeatMessage(uint64_t key, datatypes::XbotHeader *header,
                              const uint8_t *payload, size_t payload_len);
  void HandleConfigurationRequest(uint64_t key, datatypes::XbotHeader *header,
                                  const uint8_t *payload, size_t payload_len);
};
}  // namespace xbot::serviceif
#endif  // XBOT_FRAMEWORK_SERVICEIOIMPL_HPP
