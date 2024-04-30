//
// Created by clemens on 4/29/24.
//

#ifndef SOCKET_HPP
#define SOCKET_HPP
#include <memory>
#include <sys/types.h>
#include <string>
#include <vector>

namespace xbot::hub {
    class Socket {
    public:
        /**
         * Create a socket. This does not actually start the socket, call start() for that.
         * This is because we don't want to throw exceptions in the constructor.
         *
         * @param bind_address The address to bind to. 0.0.0.0 to bind to any interface
         * @param bind_port The port to bind to. 0 for random port.
         */
        Socket(std::string bind_address, u_int16_t bind_port);
        Socket(std::string bind_address);

        /**
         * Call to start the socket. This will actually create and bind the socket.
         * @return true on success
         */
        bool Start();

        /**
         * Join a multicast group.
         *
         * @param ip the multicast IP to join
         * @return true on success
         */
        bool JoinMulticast(std::string ip);

        /**
         * Call to receive a packet.
         * @param data The received data will be stored in the vector. The vector is cleared during this call.
         * @return true, if a packet was received
         */
        bool ReceivePacket(uint32_t &sender_ip, uint16_t &sender_port, std::vector<uint8_t> &data) const;

        /**
         * Call to transmit data
         * @param ip IP to send the data to. IP in host byte order. Use to avoid string parsing.
         * @param port The Port to send the data to.
         * @param data The data to send
         * @return true on success
         */
        bool TransmitPacket(uint32_t ip, uint16_t port, const std::vector<uint8_t> &data);

        /**
         * Call to transmit data
         * @param ip IP to send the data to.
         * @param port The Port to send the data to.
         * @param data The data to send
         * @return true on success
         */
        bool TransmitPacket(std::string ip, uint16_t port, const std::vector<uint8_t> &data);

        /**
         * Get the endpoint (IP / port) where this socket is reachable.
         * If the interface is specified as any, this will try to resolve the main network interface and return that IP.
         * Otherwise it will return the bound interface's IP.
         * @param ip return value for the ip address
         * @param port return value for the port
         * @return true on success
         */
        bool GetEndpoint(std::string &ip, uint16_t &port);

        ~Socket();

    private:
        // fd for the socket. -1 = no socket (safe to use, since socket() call will also return -1 on err)
        int fd_ = -1;

        std::string bind_ip_;
        uint16_t bind_port_;
    };
}


#endif //SOCKET_HPP
