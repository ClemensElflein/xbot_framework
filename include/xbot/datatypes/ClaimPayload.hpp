//
// Created by clemens on 3/25/24.
//

#ifndef LOGPAYLOAD_HPP
#define LOGPAYLOAD_HPP

#include <cstdint>
#include <xbot/config.hpp>
#include <xbot/datatypes/XbotHeader.hpp>

namespace xbot::datatypes {

#pragma pack(push, 1)
struct ClaimPayload {
  // Version of the protocol, increment on breaking changes.
  // 1 = initial release
  uint32_t target_ip{};
  // 8 bit message type
  uint16_t target_port{};
  // Heartbeat in micros
  uint32_t heartbeat_micros{};
} __attribute__((packed));
#pragma pack(pop)
}  // namespace xbot::datatypes

#endif  // LOGPAYLOAD_HPP
