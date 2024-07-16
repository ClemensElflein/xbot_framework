//
// Created by clemens on 4/29/24.
//

#ifndef SERVICEINFO_HPP
#define SERVICEINFO_HPP
#include <cstring>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#include "ServiceDescription.hpp"
#include "ServiceInputInfo.hpp"
#include "endpoint_utils.hpp"

namespace xbot::hub {
struct ServiceInfo {
 public:
  ServiceInfo() = default;
  // Unique ID for this service. This is used to address the service.
  // It is constructed of the fixed node ID in HEX and the serviceID.
  std::string unique_id_{};

  // Raw node and service ID for quicker comparison (without formatting it to a
  // string first)
  uint32_t node_id_[4]{};
  uint16_t service_id_{};

  // Endpoint info on where to reach the service
  uint32_t ip{};
  uint16_t port{};

  ServiceDescription description{};

  // Inputs for this service
  std::vector<ServiceInputInfo> inputs_{};

  // Constructs the ID from the member variables
  void UpdateID();
};

inline void to_json(nlohmann::json &j, const ServiceInfo &si) {
  j = nlohmann::json{{"nid", si.node_id_},
                     {"sid", si.service_id_},
                     {
                         "endpoint",
                         {{"ip", IpIntToString(si.ip)}, {"port", si.port}},
                     },
                     {"desc", si.description}};
}

inline void from_json(const nlohmann::json &j, ServiceInfo &p) {
  j.at("nid").get_to(p.node_id_);
  j.at("sid").get_to(p.service_id_);
  p.ip = IpStringToInt(j.at("endpoint").at("ip").get<std::string>());
  j.at("endpoint").at("port").get_to(p.port);
  j.at("desc").get_to(p.description);
  p.UpdateID();
}

}  // namespace xbot::hub

#endif  // SERVICEINFO_HPP
