//
// Created by clemens on 3/21/24.
//

#include <cstdlib>
#include <portable/packet.hpp>
#include <config.hpp>
#include <cstring>

xbot::comms::PacketPtr xbot::comms::allocatePacket()
{
    auto buffer = static_cast<Packet*>(malloc(sizeof(Packet)));
#ifdef DEBUG_MEM
#warning DEBUG_MEM enabled, disable for performance
    memset(buffer, 0x42, sizeof(Packet));
#endif
    // Set used data to 0, because packet is empty (byte contents might be random at this point though)
    buffer->used_data = 0;
    return buffer;
}

void xbot::comms::freePacket(PacketPtr packet_ptr)
{
    free(packet_ptr);
}

bool xbot::comms::packetAppendData(PacketPtr packet, const void* buffer, size_t size)
{
    auto packet_ptr = static_cast<Packet*>(packet);

    // Data won't fit.
    if(size + packet_ptr->used_data > config::max_packet_size)
        return false;

    memcpy(packet_ptr->buffer+packet_ptr->used_data, buffer, size);
    packet_ptr->used_data+=size;

    return true;
}
