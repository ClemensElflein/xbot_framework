//
// Created by clemens on 5/12/24.
//

#ifndef PACKET_IMPL_HPP
#define PACKET_IMPL_HPP

#include <xbot/config.hpp>

namespace xbot::service::packet {
struct Packet {
  size_t used_data;
  uint8_t buffer[xbot::config::max_packet_size];
};
}  // namespace xbot::service::packet

#define XBOT_PACKET_TYPEDEF Packet

#endif  // PACKET_IMPL_HPP
