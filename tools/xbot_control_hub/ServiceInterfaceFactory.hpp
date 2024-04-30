//
// Created by clemens on 4/30/24.
//

#ifndef SERVICEINTERFACEFACTORY_HPP
#define SERVICEINTERFACEFACTORY_HPP
#include <map>
#include <string>

#include "ServiceDiscovery.hpp"
#include "ServiceInterface.hpp"


namespace xbot::hub {
    class ServiceInterfaceFactory : public ServiceDiscoveryCallbacks {
    public:
        bool OnServiceDiscovered(std::string uid) override;

        bool OnEndpointChanged(std::string uid) override;

        static std::shared_ptr<ServiceInterfaceFactory> GetInstance();


        ~ServiceInterfaceFactory() override = default;

    public:
        ServiceInterfaceFactory() = default;
    private:

        static std::mutex state_mutex_;
        static std::shared_ptr<ServiceInterfaceFactory> instance_;
        static std::map<std::string, ServiceInterface> interfaces_;
    };
}



#endif //SERVICEINTERFACEFACTORY_HPP
