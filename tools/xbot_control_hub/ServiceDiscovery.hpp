//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEDISCOVERY_HPP
#define SERVICEDISCOVERY_HPP
#include <map>
#include <mutex>
#include <Socket.hpp>
#include <thread>
#include "ServiceInfo.hpp"

namespace xbot::hub {

    class ServiceDiscoveryCallbacks {
    public:
        virtual ~ServiceDiscoveryCallbacks() = default;

        virtual bool OnServiceDiscovered(std::string uid) = 0;
        virtual bool OnEndpointChanged(std::string uid, uint32_t old_ip, uint16_t old_port, uint32_t new_ip, uint16_t new_port) = 0;
    };

    class ServiceDiscovery {
    public:
        // Don't allow constructing or destructing, we only ever need one instance and we need it globally.
        ServiceDiscovery() = delete;

        ~ServiceDiscovery() = delete;


        /**
         * Retrieves the IP address and port for the given service UID.
         *
         * @param uid The unique identifier of the service.
         * @param ip Reference to a variable that will store the retrieved IP address.
         * @param port Reference to a variable that will store the retrieved port.
         *
         * @return True if the endpoint for the specified UID was found and successfully retrieved, otherwise false.
         */
        static bool GetEndpoint(const std::string & uid, uint32_t &ip, uint16_t &port);

        static bool Start();

        static void RegisterCallbacks(std::shared_ptr<ServiceDiscoveryCallbacks> callbacks);

        /**
         * Gets a copy of the ServiceInfo registered for this UID.
         * @return a unique_ptr to a copy of the ServiceInfo. nullptr if none was found.
         */
        static std::unique_ptr<ServiceInfo> GetServiceInfo(const std::string &uid);

    private:
        static void Run();

        static std::recursive_mutex sd_mutex_;
        static std::map<std::string, ServiceInfo> discovered_services_;
        static std::atomic_flag stopped_;
        static std::thread sd_thread_;
        static Socket sd_socket_;
        static std::vector<std::shared_ptr<ServiceDiscoveryCallbacks>> registered_callbacks_;
    };
} // xbot::hub

#endif //SERVICEDISCOVERY_HPP
