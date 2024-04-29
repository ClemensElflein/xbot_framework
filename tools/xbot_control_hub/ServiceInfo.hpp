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
        ServiceInfo(const ServiceInfo &other)
            : unique_id_(other.unique_id_),
              ip_(other.ip_),
              port_(other.port_),
              type_(other.type_),
              version_(other.version_),
              inputs_(other.inputs_) {
        }

        ServiceInfo(ServiceInfo &&other) noexcept
            : unique_id_(std::move(other.unique_id_)),
              ip_(other.ip_),
              port_(other.port_),
              type_(std::move(other.type_)),
              version_(other.version_),
              inputs_(std::move(other.inputs_)) {
        }

        ServiceInfo & operator=(const ServiceInfo &other) {
            if (this == &other)
                return *this;
            unique_id_ = other.unique_id_;
            ip_ = other.ip_;
            port_ = other.port_;
            type_ = other.type_;
            version_ = other.version_;
            inputs_ = other.inputs_;
            return *this;
        }

        ServiceInfo & operator=(ServiceInfo &&other) noexcept {
            if (this == &other)
                return *this;
            unique_id_ = std::move(other.unique_id_);
            ip_ = other.ip_;
            port_ = other.port_;
            type_ = std::move(other.type_);
            version_ = other.version_;
            inputs_ = std::move(other.inputs_);
            return *this;
        }


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
        std::string unique_id_;

        // Endpoint info on where to reach the service
        uint32_t ip_;
        uint16_t port_;


        // Type of the service (e.g. IMU Service)
        std::string type_;

        // Version of the service (changes whenever the interface introduces breaking changes)
        uint32_t version_;

        // Inputs for this service
        std::vector<ServiceInputInfo> inputs_;
    };
} // xbot::hub

#endif //SERVICEINFO_HPP
