//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEDISCOVERY_HPP
#define SERVICEDISCOVERY_HPP
#include <map>
#include <Socket.hpp>
#include <thread>
#include "ServiceInfo.hpp"

namespace xbot::hub {
    class ServiceDiscovery {
    public:
        // Don't allow constructing or destructing, we only ever need one instance and we need it globally.
        ServiceDiscovery() = delete;

        ~ServiceDiscovery() = delete;

        static bool Start();

    private:
        static void Run();

        static std::mutex discovered_services_mutex_;
        static std::map<std::string, ServiceInfo> discovered_services_;
        static std::thread sd_thread_;
        static Socket sd_socket_;
    };
} // xbot::hub

#endif //SERVICEDISCOVERY_HPP
