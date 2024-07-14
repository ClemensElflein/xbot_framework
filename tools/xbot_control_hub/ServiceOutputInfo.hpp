//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEOUTPUTINFO_HPP
#define SERVICEOUTPUTINFO_HPP
#include <string>
#include <utility>

namespace xbot::hub {

class ServiceOutputInfo {
 public:
  ServiceOutputInfo(uint16_t id, std::string name, std::string type)
      : id_(id), name_(std::move(name)), type_(std::move(type)) {}

  // ID of the output (used for sending data to it)
  const uint16_t id_;

  // Name of the output channel (e.g. "acceleration")
  const std::string name_;

  // Type of the output channel (e.g. uint32_t or char[100])
  const std::string type_;
};

}  // namespace xbot::hub

#endif  // SERVICEOUTPUTINFO_HPP
