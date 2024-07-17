//
// Created by clemens on 3/20/24.
//

#ifndef PACKET_HPP
#define PACKET_HPP
#include <xbot/packet_impl.hpp>

#ifndef XBOT_PACKET_TYPEDEF
#error XBOT_PACKET_TYPEDEF undefined
#endif

namespace xbot::service::packet {

typedef XBOT_PACKET_TYPEDEF* PacketPtr;

/**
 * @brief Allocate a packet.
 *
 * @return PacketPtr A pointer to the allocated packet.
 *
 * Allocates a packet and returns a pointer to it. This will block until a
 * packet is available.
 */
PacketPtr allocatePacket();

/**
 * @brief Free the memory used by a packet.
 *
 * This function frees the memory allocated for a packet.
 *
 * @param packet_ptr A pointer to the packet to be freed.
 */
void freePacket(PacketPtr packet_ptr);

bool packetAppendData(PacketPtr packet, const void* buffer, size_t size);

bool packetGetData(PacketPtr packet, void** buffer, size_t* size);

}  // namespace xbot::service::packet

#endif  // PACKET_HPP
