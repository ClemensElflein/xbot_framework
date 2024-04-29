//
// Created by clemens on 3/20/24.
//

#ifndef PACKET_HPP
#define PACKET_HPP
#include <cstdint>
#include <xbot/config.hpp>

namespace xbot::comms
{
    struct Packet
    {
        size_t used_data;
        uint8_t buffer[config::max_packet_size];
    };

    typedef Packet* PacketPtr;

    /**
     * @brief Allocate a packet.
     *
     * @return PacketPtr A pointer to the allocated packet.
     *
     * Allocates a packet and returns a pointer to it. This will block until a packet is available.
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
}

#endif //PACKET_HPP
