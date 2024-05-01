//
// Created by clemens on 4/30/24.
//

#ifndef SERVICEINTERFACEFACTORY_HPP
#define SERVICEINTERFACEFACTORY_HPP
#include <map>
#include <string>

#include "ServiceDiscovery.hpp"


namespace xbot::hub {
    // Keep track of the state of each service (claimed or not, timeout)
    struct ServiceState {
        // track, if we have claimed the service successfully.
        // If the service is claimed it will send its outputs to this interface.
        bool claimed_successfully_{false};

        // track when we sent the last claim, so that we don't spam the service
        std::chrono::time_point<std::chrono::steady_clock> last_claim_sent_{std::chrono::seconds(0)};
        std::chrono::time_point<std::chrono::steady_clock> last_heartbeat_received_{std::chrono::seconds(0)};
    };
    class ServiceInterfaceFactory : public ServiceDiscoveryCallbacks {
    public:
        bool OnServiceDiscovered(std::string uid) override;

        bool OnEndpointChanged(std::string uid, uint32_t old_ip, uint16_t old_port, uint32_t new_ip, uint16_t new_port) override;

        static std::shared_ptr<ServiceInterfaceFactory> GetInstance();

        static bool Start();

        // Deleted copy constructor
        ServiceInterfaceFactory(ServiceInterfaceFactory &other) = delete;

        // Deleted assignment operator
        void operator=(const ServiceInterfaceFactory &) = delete;

        ServiceInterfaceFactory() = default;
        ~ServiceInterfaceFactory() override = default;
    private:



        static inline uint64_t BuildKey(uint32_t ip, uint16_t port) {
            return static_cast<uint64_t>(ip) << 32 | port;
        }

        static void RunIo();

        static void ClaimService(uint64_t key);

        static bool TransmitPacket(uint64_t key, const std::vector<uint8_t> &data);

        /**
         * Maps endpoint to state. Pointers are used so that we can move the state for cheap once the endpoint changes.
         */
        static std::map<uint64_t, std::unique_ptr<ServiceState>> endpoint_map_;

        // Protects endpoint_map_ and instance_
        static std::recursive_mutex state_mutex_;
        static std::shared_ptr<ServiceInterfaceFactory> instance_;

        static std::thread io_thread_;
        static Socket io_socket_;
        static std::atomic_flag stopped_;
        // track when we last checked for claims and timeouts
        static std::chrono::time_point<std::chrono::steady_clock> last_check_;

    };
}



#endif //SERVICEINTERFACEFACTORY_HPP
