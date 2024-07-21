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
   * Called whenever a new transaction starts
   */
  virtual void OnTransactionStart(uint64_t timestamp) = 0;

  /**
   * Called whenever a transaction was finished
   */
  virtual void OnTransactionEnd() = 0;

  /**
   * Called whenever a packet is received from the specified service.
   * @param uid service uid
   * @param timestamp timestamp
   * @param target_id ID of the io target
   * @param payload the raw payload
   * @param buflen size of the payload buffer
   */
  virtual void OnData(const std::string &uid, uint64_t timestamp,
                      uint16_t target_id, const void *payload,
                      size_t buflen) = 0;

  /**
   * Called whenever a service needs configuration. Whenever this is called,
   * send a configuration transaction to the requesting service
   * @param uid service uid
   */
  virtual bool OnConfigurationRequested(const std::string &uid) = 0;

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

  /**
   * Send data to a given service target
   */
  static bool SendData(const std::string &uid,
                       const std::vector<uint8_t> &data);

  ServiceIO() = default;
  ~ServiceIO() override = default;

 private:
  static inline uint64_t BuildKey(uint32_t ip, uint16_t port) {
    return static_cast<uint64_t>(ip) << 32 | port;
  }

  static void RunIo();

  static void ClaimService(uint64_t key);

  static bool TransmitPacket(uint64_t key, const std::vector<uint8_t> &data);

  static void HandleClaimMessage(uint64_t key, datatypes::XbotHeader *header,
                                 const uint8_t *payload, size_t payload_len);
  static void HandleDataMessage(uint64_t key, datatypes::XbotHeader *header,
                                const uint8_t *payload, size_t payload_len);
  static void HandleDataTransaction(uint64_t key, datatypes::XbotHeader *header,
                                    const uint8_t *payload, size_t payload_len);
  static void HandleHeartbeatMessage(uint64_t key,
                                     datatypes::XbotHeader *header,
                                     const uint8_t *payload,
                                     size_t payload_len);
  static void HandleConfigurationRequest(uint64_t key,
                                         datatypes::XbotHeader *header,
                                         const uint8_t *payload,
                                         size_t payload_len);
};
}  // namespace xbot::serviceif

#endif  // SERVICEINTERFACEFACTORY_HPP
