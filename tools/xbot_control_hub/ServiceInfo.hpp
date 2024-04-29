//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEINFO_HPP
#define SERVICEINFO_HPP
#include <string>
#include <vector>

#include "ServiceInputInfo.hpp"

namespace xbot::hub {
    class ServiceInfo {
    public:
        ServiceInfo(std::string unique_id, uint32_t ip, uint16_t port, std::string type, uint32_t version,
            std::vector<ServiceInputInfo> inputs)
            : unique_id_(std::move(unique_id)),
              ip_(ip),
              port_(port),
              type_(std::move(type)),
              version_(version),
              inputs_(std::move(inputs)) {
        }


        // Unique ID for this service. This is used to address the service.
        // It is constructed of the fixed node ID in HEX and the serviceID.
        const std::string unique_id_;

        // Endpoint info on where to reach the service
        const uint32_t ip_;
        const uint16_t port_;


        // Type of the service (e.g. IMU Service)
        const std::string type_;

        // Version of the service (changes whenever the interface introduces breaking changes)
        const uint32_t version_;

        // Inputs for this service
        const std::vector<ServiceInputInfo> inputs_;
    };
} // xbot::hub

#endif //SERVICEINFO_HPP
