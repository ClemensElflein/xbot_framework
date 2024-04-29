//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEINPUTINFO_HPP
#define SERVICEINPUTINFO_HPP
#include <string>
#include <utility>


namespace xbot::hub {

class ServiceInputInfo {
public:
    ServiceInputInfo(uint16_t id, std::string name, std::string type)
        : id_(id),
          name_(std::move(name)),
          type_(std::move(type)) {
    }

    // ID of the input (used for sending data to it)
    const uint16_t id_;

    // Name of the input channel (e.g. "speed")
    const std::string name_;

    // Type of the input channel (e.g. uint32_t or char[100])
    const std::string type_;
};

} // xbot::hub

#endif //SERVICEINPUTINFO_HPP
