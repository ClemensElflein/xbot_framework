//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEDISCOVERYIMPL_HPP
#define SERVICEDISCOVERYIMPL_HPP
#include <map>
#include <mutex>
#include <thread>
#include <xbot-service-interface/ServiceDiscovery.hpp>
#include <xbot-service-interface/data/ServiceInfo.hpp>

namespace xbot::serviceif {

class ServiceDiscoveryImpl : public ServiceDiscovery {
 public:
  ServiceDiscoveryImpl() = default;

  ~ServiceDiscoveryImpl() = default;

  /**
   * Retrieves the IP address and port for the given service UID.
   *
   * @param uid The unique identifier of the service.
   * @param ip Reference to a variable that will store the retrieved IP address.
   * @param port Reference to a variable that will store the retrieved port.
   *
   * @return True if the endpoint for the specified UID was found and
   * successfully retrieved, otherwise false.
   */
  bool GetEndpoint(const std::string &uid, uint32_t &ip, uint16_t &port);

  bool Start();
  bool Stop();

  void RegisterCallbacks(ServiceDiscoveryCallbacks *callbacks) override;
  void UnregisterCallbacks(ServiceDiscoveryCallbacks *callbacks) override;

  /**
   * Gets a copy of the ServiceInfo registered for this UID.
   * @return a unique_ptr to a copy of the ServiceInfo. nullptr if none was
   * found.
   */
  std::unique_ptr<ServiceInfo> GetServiceInfo(const std::string &uid) override;

  /**
   * Get a copy of registered services
   */
  std::unique_ptr<std::map<std::string, ServiceInfo>> GetAllServices();

  /**
   * Drops a service from the ServiceDiscovery list.
   * Use this, whenever a service times out in order to get re-notified
   * when this service comes back up.
   * @param uid The services unique ID
   */
  bool DropService(const std::string &uid);

  static ServiceDiscoveryImpl *GetInstance();
 static void SetBindIp(std::string bind_ip);
};
}  // namespace xbot::serviceif

#endif  // SERVICEDISCOVERY_HPP
