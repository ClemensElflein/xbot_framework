//
// Created by clemens on 4/30/24.
//

#ifndef SERVICEINTERFACEFACTORY_HPP
#define SERVICEINTERFACEFACTORY_HPP

#include <string>
#include <xbot-service-interface/ServiceDiscovery.hpp>
#include <xbot/datatypes/XbotHeader.hpp>

namespace xbot::serviceif {

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
class ServiceIO {
 public:
  /**
   * Register callbacks for a specific uid
   * @param uid the UID to listen to
   * @param callbacks pointer to the callbacks
   */
  virtual void RegisterCallbacks(const std::string &uid,
                                 ServiceIOCallbacks *callbacks) = 0;

  /**
   * Unregister callbacks for all uids
   * @param callbacks callback pointer
   */
  virtual void UnregisterCallbacks(ServiceIOCallbacks *callbacks) = 0;

  /**
   * Send data to a given service target
   */
  virtual bool SendData(const std::string &uid,
                        const std::vector<uint8_t> &data) = 0;

  /**
   * Call this to check if IO is still running.
   * On shutdown this will return false, stop your interface then
   */
  virtual bool OK() = 0;
};
}  // namespace xbot::serviceif

#endif  // SERVICEINTERFACEFACTORY_HPP
