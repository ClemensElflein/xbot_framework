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
  ServiceInputInfo(const ServiceInputInfo &other)
      : id_(other.id_), name_(other.name_), type_(other.type_) {}

  ServiceInputInfo(ServiceInputInfo &&other) noexcept
      : id_(other.id_),
        name_(std::move(other.name_)),
        type_(std::move(other.type_)) {}

  ServiceInputInfo &operator=(const ServiceInputInfo &other) {
    if (this == &other) return *this;
    id_ = other.id_;
    name_ = other.name_;
    type_ = other.type_;
    return *this;
  }

  ServiceInputInfo &operator=(ServiceInputInfo &&other) noexcept {
    if (this == &other) return *this;
    id_ = other.id_;
    name_ = std::move(other.name_);
    type_ = std::move(other.type_);
    return *this;
  }

  ServiceInputInfo(uint16_t id, std::string name, std::string type)
      : id_(id), name_(std::move(name)), type_(std::move(type)) {}

  // ID of the input (used for sending data to it)
  uint16_t id_;

  // Name of the input channel (e.g. "speed")
  std::string name_;

  // Type of the input channel (e.g. uint32_t or char[100])
  std::string type_;
};

}  // namespace xbot::hub

#endif  // SERVICEINPUTINFO_HPP
