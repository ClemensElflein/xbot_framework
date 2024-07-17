//
// Created by clemens on 3/25/24.
//

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

namespace xbot::service::system {
void initSystem(uint32_t node_id = 0);
uint32_t getTimeMicros();
bool getNodeId(uint8_t *id, size_t id_len);
}  // namespace xbot::service::system
#endif  // SYSTEM_HPP
