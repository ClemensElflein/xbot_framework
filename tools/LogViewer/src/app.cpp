//
// Created by clemens on 3/21/24.
//
#include <condition_variable>
#include <csignal>

#include <iostream>
#include <thread>
#include <semaphore>
#include <ulog.h>
#include <etl/reference_flat_map.h>
#include <etl/flat_map.h>

#include "datatypes/LogPayload.hpp"
#include "datatypes/ServiceDiscovery.hpp"
#include "datatypes/XbotHeader.hpp"
#include "portable/socket.hpp"
#include "portable/system.hpp"
#include "portable/thread.hpp"



void console_logger(ulog_level_t severity, char *msg, void *arg) {
    printf("![LogViewer] [%s]: %s\n",
        ulog_level_name(severity),
        msg);
}


std::binary_semaphore sema{0};
// Signal handler function
void handleSignal(int signal)
{
    if(signal == SIGINT)
    {
        std::cout << "got SIGINT. Shutting Down." << std::endl;
        sema.release();
    }
}


void* log_thread() {

    xbot::comms::SocketPtr log_socket = xbot::comms::createSocket(true);
    xbot::comms::subscribeMulticast(log_socket, xbot::comms::config::remote_log_multicast_address);
    xbot::comms::subscribeMulticast(log_socket, xbot::comms::config::sd_multicast_address);

    while (true) {
        xbot::comms::PacketPtr packet;
        if(xbot::comms::receivePacket(log_socket, &packet)) {
            // Got a packet, check if valid and put it into the processing queue.
            if(packet->used_data < sizeof(xbot::comms::datatypes::XbotHeader)) {
                ULOG_ERROR("Packet too short to contain header.");
                freePacket(packet);
                continue;
            }

            const auto header = reinterpret_cast<xbot::comms::datatypes::XbotHeader*>(packet->buffer);
            // Check, if the header size is correct
            if(packet->used_data - sizeof(xbot::comms::datatypes::XbotHeader) != header->payload_size) {
                // TODO: In order to allow chaining of xBot packets in the future, this needs to be adapted.
                // (scan and split packets)
                ULOG_ERROR("Packet header size does not match actual packet size.");
                freePacket(packet);
                continue;
            }

            if(header->message_type == xbot::comms::datatypes::MessageType::LOG) {
                xbot::comms::datatypes::LogMessage msg;
                if(msg.load(packet->buffer+sizeof(xbot::comms::datatypes::XbotHeader)).status != serdes::status_e::NO_ERROR) {
                    ULOG_ERROR("Error deserializing packet");
                } else {
                    // We got a log packet, log it
                    std::cout << "message: " << msg.message << std::endl;
                    // ULOG_WARNING(msg.message);
                }
            } else if(header->message_type == xbot::comms::datatypes::MessageType::SERVICE_ADVERTISEMENT) {
                std::cout << "got service advertisement:" << std::endl;
                xbot::comms::datatypes::SDServiceAdvertisement msg;
                auto serdes_result = msg.load(packet->buffer+sizeof(xbot::comms::datatypes::XbotHeader));
                if(serdes_result.status != serdes::status_e::NO_ERROR) {
                    ULOG_ERROR("Error deserializing packet");
                } else {
                    // We got a log packet, log it
                    std::cout << "service id: " << header->service_id << std::endl;
                    std::cout << "entry count: " << msg.entry_count << std::endl;
                    for(uint16_t i = 0; i < msg.entry_count; i++) {
                        xbot::comms::datatypes::SDServiceInfoEntry entry;
                        serdes_result = entry.load(packet->buffer+sizeof(xbot::comms::datatypes::XbotHeader), sizeof(packet->buffer), serdes_result.bits);
                        if(serdes_result.status != serdes::status_e::NO_ERROR) {
                            ULOG_ERROR("Error deserializing packet");
                        } else {
                            std::cout << entry.name << std::endl;
                            switch (entry.entry_type) {
                                case xbot::comms::datatypes::SDServiceInfoEntryType::UNKNOWN:
                                    std::cout << "-- entry type: " << "UNKNOWN" << std::endl;
                                    break;
                                case xbot::comms::datatypes::SDServiceInfoEntryType::SERVICE_NAME:
                                    std::cout << "-- entry type: " << "SERVICE_NAME" << std::endl;
                                    break;
                                case xbot::comms::datatypes::SDServiceInfoEntryType::INPUT_CHANNEL:
                                    std::cout << "-- entry type: " << "INPUT_CHANNEL" << std::endl;
                                    break;
                                case xbot::comms::datatypes::SDServiceInfoEntryType::OUTPUT_CHANNEL:
                                    std::cout << "-- entry type: " << "OUTPUT_CHANNEL" << std::endl;
                                    break;
                                case xbot::comms::datatypes::SDServiceInfoEntryType::PROPERTY:
                                    std::cout << "-- entry type: " << "PROPERTY" << std::endl;
                                    break;
                                default:
                                    std::cout << "-- entry type: " << "INVALID" << std::endl;
                            }
                        }
                    }
                    // ULOG_WARNING(msg.message);
                }
            }

            xbot::comms::freePacket(packet);
        }
    }
}


int main()
{
    etl::flat_map data{ etl::pair{0, 1}, etl::pair{2, 3}, etl::pair{4, 5}, etl::pair{6, 7} };
    initSystem();
    ULOG_SUBSCRIBE(console_logger, ULOG_WARNING_LEVEL);
    std::thread thread{log_thread};

    // Set up signal handling function for SIGINT (CTRL+C)
    std::signal(SIGINT, handleSignal);
    // Wait for signal
    sema.acquire();

    thread.join();

    return 0;
}
