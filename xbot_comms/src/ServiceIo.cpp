//
// Created by clemens on 7/14/24.
//
#include <ServiceIo.h>
#include <ulog.h>

#include <xbot/datatypes/XbotHeader.hpp>

namespace xbot::comms {

ServiceIo::ServiceIo(uint32_t service_id)
    : service_id_(service_id), next_service_(nullptr) {}

bool ServiceIo::ioInput(packet::PacketPtr packet) {
  // Got a packet, check if valid and put it into the processing queue.
  void* buffer = nullptr;
  size_t used_data = 0;
  if (!packet::packetGetData(packet, &buffer, &used_data)) {
    packet::freePacket(packet);
    return false;
  }
  if (used_data < sizeof(datatypes::XbotHeader)) {
    ULOG_ARG_ERROR(&service_id_, "Packet too short to contain header.");
    packet::freePacket(packet);
    return false;
  }

  const auto header = reinterpret_cast<datatypes::XbotHeader*>(buffer);
  // Check, if the header size is correct
  if (used_data - sizeof(datatypes::XbotHeader) != header->payload_size) {
    // TODO: In order to allow chaining of xBot packets in the future,
    // this needs to be adapted. (scan and split packets)
    ULOG_ARG_ERROR(&service_id_,
                   "Packet header size does not match actual packet size.");
    packet::freePacket(packet);
    return false;
  }

  if (!queue::queuePushItem(&packet_queue_, packet)) {
    ULOG_ARG_ERROR(&service_id_, "Error pushing packet into processing queue.");
    packet::freePacket(packet);
    return false;
  }
}
}  // namespace xbot::comms