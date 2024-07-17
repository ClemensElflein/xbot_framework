//
// Created by clemens on 4/30/24.
//

#ifndef SERVICEINTERFACEFACTORY_HPP
#define SERVICEINTERFACEFACTORY_HPP
#include <string>
#include <xbot/datatypes/XbotHeader.hpp>

#include "ServiceDiscovery.hpp"

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

class ServiceIOCallbacks {
 public:
  virtual ~ServiceIOCallbacks() = default;

  /**
   * Called whenever a service is connected.
   * Connected means that it was discovered and claimed successfully.
   */
  virtual void OnServiceConnected(const std::string &uid) = 0;

  /**
   * Called whenever a packet is received from the specified service.
   * @param service service uid
   * @param header the header for the received packet
   * @param payload the raw payload
   */
  virtual void OnData(const std::string &uid,
                      const datatypes::XbotHeader &header,
                      const void *payload) = 0;

  /**
   * Called whenever a service is disconnected.
   * This could be due to timeout. After this OnData won't be called anymore
   * with the uid. Note that OnServiceConnected might be called on reconnection.
   * Then OnData will be called again as expected.
   * @param uid the service's uid
   */
  virtual void OnServiceDisconnected(const std::string &uid) = 0;
};

/**
 * ServiceIO subscribes to ServiceDiscovery and claims all services anyone
 * is interested in. It keeps track of the timeouts and redirects the actual
 * data to the actual subscribers.
 */
class ServiceIO : public ServiceDiscoveryCallbacks {
 public:
  bool OnServiceDiscovered(std::string uid) override;

  bool OnEndpointChanged(std::string uid, uint32_t old_ip, uint16_t old_port,
                         uint32_t new_ip, uint16_t new_port) override;

  static ServiceIO *GetInstance();

  static bool Start();

  // Deleted copy constructor
  ServiceIO(ServiceIO &other) = delete;

  // Deleted assignment operator
  void operator=(const ServiceIO &) = delete;

  /**
   * Register callbacks for a specific uid
   * @param uid the UID to listen to
   * @param callbacks pointer to the callbacks
   */
  static void RegisterCallbacks(const std::string &uid,
                                ServiceIOCallbacks *callbacks);

  /**
   * Unregister callbacks for all uids
   * @param callbacks callback pointer
   */
  static void UnregisterCallbacks(ServiceIOCallbacks *callbacks);

  ServiceIO() = default;
  ~ServiceIO() override = default;

 private:
  static inline uint64_t BuildKey(uint32_t ip, uint16_t port) {
    return static_cast<uint64_t>(ip) << 32 | port;
  }

  static void RunIo();

  static void ClaimService(uint64_t key);

  static bool TransmitPacket(uint64_t key, const std::vector<uint8_t> &data);
};
}  // namespace xbot::serviceif

#endif  // SERVICEINTERFACEFACTORY_HPP
