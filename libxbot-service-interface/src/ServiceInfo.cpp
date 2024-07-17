//
// Created by clemens on 4/30/24.
//

#include <cstring>
#include <iomanip>
#include <string>
#include <xbot-service-interface/data/ServiceInfo.hpp>

using namespace xbot::serviceif;

void ServiceInfo::UpdateID() {
  std::stringstream stream;
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[0];
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[1];
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[2];
  stream << std::hex << std::setw(8) << std::setfill('0') << node_id_[3];
  stream << ":";
  stream << std::hex << std::setw(4) << std::setfill('0') << service_id_;
  unique_id_ = stream.str();
}
