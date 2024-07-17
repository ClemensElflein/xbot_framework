//
// Created by clemens on 3/25/24.
//

#include <ulog.h>

#include <cstdio>
#include <cstring>
#include <xbot-service/Lock.hpp>
#include <xbot-service/RemoteLogging.hpp>
#include <xbot-service/portable/socket.hpp>
#include <xbot/config.hpp>

#include "xbot/datatypes/LogPayload.hpp"
#include "xbot/datatypes/XbotHeader.hpp"

using namespace xbot::service;
using namespace xbot::datatypes;

XBOT_MUTEX_TYPEDEF logging_mutex{};
XBOT_SOCKET_TYPEDEF logging_socket{};
uint8_t log_packet_buffer[xbot::config::max_packet_size];

XbotHeader log_message_header{};

uint16_t log_sequence_no = 0;

void remote_logger(ulog_level_t severity, char* msg, const void* args) {
  Lock lk(&logging_mutex);

  // Packet Header
  log_message_header.protocol_version = 0;
  log_message_header.message_type = MessageType::LOG;
  log_message_header.flags = 0;

  log_message_header.arg1 = (severity - ULOG_TRACE_LEVEL + 1);
  log_message_header.arg2 = 0;
  log_message_header.sequence_no = log_sequence_no++;
  log_message_header.timestamp = 0;

  int written = snprintf(msg, xbot::config::max_log_length, "[%s]: %s",
                         ulog_level_name(severity), msg);

  if (written > 0 && written < xbot::config::max_log_length) {
    // Need to add that terminating 0, for cppserdes to copy it.
    log_message_header.payload_size = written + 1;
    // serdes::status_t result = log_message.store(log_packet_buffer);
    // if(result.status == serdes::status_e::NO_ERROR) {
    // (result.bits+7)/8 is effectively ceil(result.bits/8) without the float
    // log_message_header.payload_size = (result.bits+7)/8;
    // Serialize success, send it.
    // PacketPtr log_packet = allocatePacket();
    // packetAppendData(log_packet, &log_message_header,
    // sizeof(log_message_header)); packetAppendData(log_packet,
    // &log_packet_buffer, log_message_header.payload_size);
    // socketTransmitPacket(logging_socket, log_packet,
    // config::remote_log_multicast_address, config::multicast_port);
    // }
  }
}

bool startRemoteLogging() {
  if (!mutex::initialize(&logging_mutex)) {
    return false;
  }

  Lock lk(&logging_mutex);
  if (!sock::initialize(&logging_socket, false)) {
    ULOG_ERROR("Error setting up remote logging: Error creating socket");
    return false;
  }

  ULOG_SUBSCRIBE(remote_logger, ULOG_INFO_LEVEL);
  return true;
}
