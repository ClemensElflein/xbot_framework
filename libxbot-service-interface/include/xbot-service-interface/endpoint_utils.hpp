//
// Created by clemens on 4/30/24.
//

#ifndef ENDPOINT_UTILS_HPP
#define ENDPOINT_UTILS_HPP
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cstdint>
#include <string>

namespace xbot::serviceif {
inline uint32_t IpStringToInt(const std::string& ip_string) {
  uint32_t ip = 0xFFFFFFFF;
  inet_pton(AF_INET, ip_string.c_str(), &ip);
  ip = ntohl(ip);
  return ip;
}

inline std::string IpIntToString(const uint32_t ip) {
  in_addr addr{.s_addr = ntohl(ip)};
  const char* addrStr = inet_ntoa(addr);
  return addrStr;
}

inline std::string EndpointIntToString(const uint32_t ip, const uint16_t port) {
  return IpIntToString(ip) + ":" + std::to_string(port);
}
}  // namespace xbot::serviceif

#endif  // ENDPOINT_UTILS_HPP
