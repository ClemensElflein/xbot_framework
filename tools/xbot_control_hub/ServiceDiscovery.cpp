//
// Created by clemens on 4/29/24.
//

#include "ServiceDiscovery.hpp"
#include <iostream>
#include <mutex>
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
        if (!sd_socket_.Start())
            return false;

        if (!sd_socket_.JoinMulticast(config::sd_multicast_address))
            return false;


        sd_thread_ = std::thread{ServiceDiscovery::Run};
        return true;
    }

    void ServiceDiscovery::Run() {
        std::vector<uint8_t> packet{};
        uint32_t sender_ip;
        uint16_t sender_port;
        while (true) {
            if (sd_socket_.ReceivePacket(sender_ip, sender_port, packet)) {
                struct in_addr addr;
                addr.s_addr = sender_ip;
                std::cout << "Got packet from " << inet_ntoa(addr) << " with size " << packet.size() << "!" <<
                        std::endl;

                if (packet.size() >= sizeof(comms::datatypes::XbotHeader)) {
                    const auto header = reinterpret_cast<comms::datatypes::XbotHeader *>(packet.data());

                    // Validate reported length
                    if (packet.size() == header->payload_size + sizeof(comms::datatypes::XbotHeader)) {
                        try {
                            const auto json = nlohmann::json::from_cbor(
                                packet.begin() + sizeof(xbot::comms::datatypes::XbotHeader), packet.end());

                            std::stringstream stream;
                            stream << std::hex << std::setw(8) << std::setfill('0') << json.at("nid").at(0).get<
                                uint32_t>();
                            stream << std::hex << std::setw(8) << std::setfill('0') << json.at("nid").at(1).get<
                                uint32_t>();
                            stream << std::hex << std::setw(8) << std::setfill('0') << json.at("nid").at(2).get<
                                uint32_t>();
                            stream << std::hex << std::setw(8) << std::setfill('0') << json.at("nid").at(3).get<
                                uint32_t>();
                            stream << ":";
                            stream << std::hex << std::setw(4) << std::setfill('0') << json.at("sid").get<uint16_t>();
                            std::string uid = stream.str();

                            std::cout << "found service with ID " << uid << std::endl;

                            // Convert endpoint string to int
                            uint32_t ip = 0xFFFFFFFF;
                            inet_pton(AF_INET, json.at("endpoint").at("ip").get<std::string>().c_str(), &ip);
                            ip = ntohl(ip);

                            std::vector<ServiceInputInfo> inputs{};
                            ServiceInfo info{
                                uid, ip, json.at("endpoint").at("port").get<uint16_t>(),
                                json.at("desc").at("type").get<std::string>(),
                                json.at("desc").at("version").get<uint32_t>(), inputs
                            };

                            {
                                std::unique_lock lk(discovered_services_mutex_);
                                if(discovered_services_.contains(uid))
                                    discovered_services_.erase(uid);
                                discovered_services_.emplace(uid, info);
                            }
                        } catch (std::exception &e) {
                            std::cout << "got exception " << e.what() << std::endl;
                        }
                    }
                }
            }
        }
    }
} // xbot::hub
