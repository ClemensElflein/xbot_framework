//
// Created by clemens on 4/30/24.
//

#include "ServiceInfo.hpp"

#include <cstring>
#include <iomanip>
#include <string>

using namespace xbot::hub;

ServiceInfo::ServiceInfo(const uint32_t node_id[4], const uint16_t service_id,
                         const uint32_t ip, const uint16_t port,
                         std::string type, const uint32_t version,
                         std::vector<ServiceInputInfo> inputs)
    : service_id_(service_id),
      ip_(ip),
      port_(port),
      type_(std::move(type)),
      version_(version),
      inputs_(std::move(inputs)) {
  memcpy(node_id_, node_id, sizeof(node_id_));
  std::stringstream stream;
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[0];
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[1];
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[2];
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[3];
  stream << ":";
  stream << std::hex << std::setw(4) << std::setfill('0') << service_id;
  unique_id_ = stream.str();
}
