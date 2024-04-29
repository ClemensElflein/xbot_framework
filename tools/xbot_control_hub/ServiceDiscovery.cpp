//
// Created by clemens on 4/29/24.
//

#include "ServiceDiscovery.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <xbot/config.hpp>
#include <xbot/datatypes/XbotHeader.hpp>
#include <nlohmann/json.hpp>

namespace xbot::hub {
    Socket ServiceDiscovery::sd_socket_{"0.0.0.0", config::multicast_port};
    std::thread ServiceDiscovery::sd_thread_{};
    std::mutex ServiceDiscovery::discovered_services_mutex_{};
    std::map<std::string, ServiceInfo> ServiceDiscovery::discovered_services_{};

    bool ServiceDiscovery::Start() {
        if(!sd_socket_.Start())
            return false;

        if(!sd_socket_.JoinMulticast(config::sd_multicast_address))
            return false;


        sd_thread_ = std::thread{ServiceDiscovery::Run};
        return true;
    }

    void ServiceDiscovery::Run() {
        std::vector<uint8_t> packet{};
        uint32_t sender_ip;
        uint16_t sender_port;
        while (true) {
            if(sd_socket_.ReceivePacket(sender_ip, sender_port, packet)) {
                struct in_addr addr;
                addr.s_addr = sender_ip;
                std::cout << "Got packet from "<< inet_ntoa(addr) << " with size "<< packet.size() << "!" << std::endl;

                if(packet.size() >= sizeof(comms::datatypes::XbotHeader)) {
                    const auto header = reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());

                    // Validate reported length
                    if(packet.size() == header->payload_size + sizeof(comms::datatypes::XbotHeader)) {
                        try {
                            for(auto iter = packet.begin()+sizeof(xbot::comms::datatypes::XbotHeader); iter != packet.end(); ++iter) {
                                printf("%02x", *iter);
                            }
                            const auto json = nlohmann::json::from_cbor(packet.begin()+sizeof(xbot::comms::datatypes::XbotHeader), packet.end());



                            std::cout << json << std::endl;
                        } catch (std::exception &e) {
                            std::cout << "got exception " << e.what() << std::endl;
                        }

                    }


                }


            }
        }
    }
} // xbot::hub