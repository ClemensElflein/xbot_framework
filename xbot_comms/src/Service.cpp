//
// Created by clemens on 3/20/24.
//
#include <algorithm>
#include <Service.hpp>
#include <ulog.h>
#include <xbot/datatypes/ClaimPayload.hpp>

#include "Lock.hpp"
#include "portable/system.hpp"

uint8_t xbot::comms::Service::sd_buffer[] = {};
xbot::comms::MutexPtr xbot::comms::Service::sd_buffer_mutex = createMutex();

xbot::comms::Service::Service(uint16_t service_id, uint32_t tick_rate_micros) :
    tick_rate_micros_(tick_rate_micros),
    service_id_(service_id),
    udp_socket_(createSocket(false)),
    state_mutex_(createMutex()),
    packet_queue_ptr_(createQueue(10))
{
    // Set reboot flag
    header_.flags = 1;
}

xbot::comms::Service::~Service()
{
    deleteSocket(udp_socket_);
    deleteMutex(state_mutex_);
    deleteThread(process_thread_);
}

bool xbot::comms::Service::start()
{
    stopped = false;

    process_thread_ = createThread(Service::startProcessingHelper, this);
    io_thread_ = createThread(Service::startIoHelper, this);

    // Error creating thread
    return process_thread_ != nullptr;
}

bool xbot::comms::Service::SendData(uint16_t target_id, const void *data, size_t size) {
    if(target_ip == 0 || target_port == 0) {
        ULOG_ARG_INFO(&service_id_, "Service has no target, dropping packet");
        return false;
    }
    fillHeader();
    header_.message_type = datatypes::MessageType::DATA;
    header_.payload_size = size;
    header_.arg2 = target_id;

    // Send header and data
    PacketPtr ptr = allocatePacket();
    packetAppendData(ptr, &header_, sizeof(header_));
    packetAppendData(ptr, data, size);
    return socketTransmitPacket(udp_socket_, ptr, target_ip, target_port);
}

bool xbot::comms::Service::SendDataClaimAck() {
    if(target_ip == 0 || target_port == 0) {
        ULOG_ARG_INFO(&service_id_, "Service has no target, dropping packet");
        return false;
    }
    fillHeader();
    header_.message_type = datatypes::MessageType::CLAIM;
    header_.payload_size = 0;
    header_.arg1 = 1;

    // Send header and data
    PacketPtr ptr = allocatePacket();
    packetAppendData(ptr, &header_, sizeof(header_));
    return socketTransmitPacket(udp_socket_, ptr, target_ip, target_port);
}

void xbot::comms::Service::runIo() {
    while(true) {
        // Check, if we should stop
        {
            Lock lk(state_mutex_);
            if (stopped)
            {
                return;
            }
        }

        PacketPtr packet = nullptr;
        if(receivePacket(udp_socket_, &packet)) {
            // Got a packet, check if valid and put it into the processing queue.
            if(packet->used_data < sizeof(datatypes::XbotHeader)) {
                ULOG_ARG_ERROR(&service_id_, "Packet too short to contain header.");
                freePacket(packet);
                continue;
            }

            const auto header = reinterpret_cast<datatypes::XbotHeader*>(packet->buffer);
            // Check, if the header size is correct
            if(packet->used_data - sizeof(datatypes::XbotHeader) != header->payload_size) {
                // TODO: In order to allow chaining of xBot packets in the future, this needs to be adapted.
                // (scan and split packets)
                ULOG_ARG_ERROR(&service_id_, "Packet header size does not match actual packet size.");
                freePacket(packet);
                continue;
            }

            if(!queuePushItem(packet_queue_ptr_, packet)) {
                ULOG_ARG_ERROR(&service_id_, "Error pushing packet into processing queue.");
                freePacket(packet);
                continue;
            }
        }
    }
}

void xbot::comms::Service::fillHeader() {
    header_.message_type = datatypes::MessageType::UNKNOWN;
    header_.payload_size = 0;
    header_.protocol_version = 1;
    header_.arg1 = 0;
    header_.arg2 = 0;
    header_.sequence_no++;
    if(header_.sequence_no == 0) {
        // Clear reboot flag on rolloger
        header_.flags &= 0xFE;
    }
    header_.timestamp = getTimeMicros();
}

void xbot::comms::Service::heartbeat() {
    if(target_ip == 0 || target_port == 0) {
        last_heartbeat_micros_ = getTimeMicros();
        return;
    }
    fillHeader();
    header_.message_type = datatypes::MessageType::HEARTBEAT;
    header_.payload_size = 0;
    header_.arg1 = 0;

    // Send header and data
    PacketPtr ptr = allocatePacket();
    packetAppendData(ptr, &header_, sizeof(header_));
    socketTransmitPacket(udp_socket_, ptr, target_ip, target_port);
    last_heartbeat_micros_ = getTimeMicros();
}

void xbot::comms::Service::runProcessing()
{
    // Check, if we should stop
    {
        Lock lk(state_mutex_);
        last_tick_micros_ = getTimeMicros();
    }
    while (true)
    {
        // Check, if we should stop
        {
            Lock lk(state_mutex_);
            if (stopped)
            {
                return;
            }
        }

        // Fetch from queue
        PacketPtr packet;
        uint32_t now_micros = getTimeMicros();
        // Calculate when the next tick needs to happen (expected tick rate - time elapsed)
        uint32_t time_to_next_tick = tick_rate_micros_-(now_micros - last_tick_micros_);
        uint32_t time_to_next_heartbeat = heartbeat_micros_-(now_micros - last_heartbeat_micros_);
        // If this is ture, we have a rollover (since we should need to wait longer than the tick length)
        if(time_to_next_tick > tick_rate_micros_)
        {
            ULOG_ARG_WARNING(&service_id_, "Service too slow to keep up with tick rate.");
            time_to_next_tick = 0;
        }
        // If this is ture, we have a rollover (since we should need to wait longer than the tick length)
        if(heartbeat_micros_ > 0 && time_to_next_heartbeat > heartbeat_micros_)
        {
            ULOG_ARG_WARNING(&service_id_, "Service too slow to keep up with heartbeat rate.");
            time_to_next_heartbeat = 0;
        }
        const uint32_t block_time = std::min(time_to_next_tick, time_to_next_heartbeat);
        if (queuePopItem(packet_queue_ptr_, reinterpret_cast<void**>(&packet), block_time))
        {
            const auto header = reinterpret_cast<datatypes::XbotHeader*>(packet->buffer);

            // TODO: filter message type, sender, ...

            if(header->message_type == datatypes::MessageType::CLAIM) {
                ULOG_ARG_INFO(&service_id_, "Received claim message");
                if(header->payload_size != sizeof(datatypes::ClaimPayload)) {
                    ULOG_ARG_ERROR(&service_id_, "claim message with invalid payload size");
                } else {
                    // it's ok, overwrite the target
                    const auto payload_ptr = reinterpret_cast<datatypes::ClaimPayload*>(packet->buffer+sizeof(datatypes::XbotHeader));
                    target_ip = payload_ptr->target_ip;
                    target_port = payload_ptr->target_port;
                    heartbeat_micros_ = payload_ptr->heartbeat_micros;

                    // Send early in order to allow for jitter
                    if(heartbeat_micros_ > config::heartbeat_jitter) {
                        heartbeat_micros_ -= config::heartbeat_jitter;
                    }

                    // send heartbeat at twice the requested rate
                    heartbeat_micros_>>=1;


                    ULOG_ARG_INFO(&service_id_, "service claimed successfully.");

                    SendDataClaimAck();
                }
            } else {
                // Packet seems OK, hand to service
                handlePacket(header, packet->buffer + sizeof(datatypes::XbotHeader));
            }


            freePacket(packet);
        }
        uint32_t now = getTimeMicros();
        // Measure time required for the tick() call, so that we can substract before next timeout
        if(last_tick_micros_ + tick_rate_micros_ < now) {
            last_tick_micros_ = now;
            tick();
        }
        if(last_service_discovery_micros_+config::sd_advertisement_interval_micros < now) {
            ULOG_ARG_DEBUG(&service_id_, "Sending SD advetisement");
            advertiseService();
            last_service_discovery_micros_ = now;
        }
        if(heartbeat_micros_ > 0 && last_heartbeat_micros_+heartbeat_micros_ < now) {
            ULOG_ARG_DEBUG(&service_id_, "Sending heartbeat");
            heartbeat();
        }
    }
}

