//
// Created by clemens on 4/30/24.
//

#ifndef SERVICEINTERFACE_HPP
#define SERVICEINTERFACE_HPP

#include <string>
#include <cstddef>
#include <Socket.hpp>
#include <thread>
#include <xbot/datatypes/XbotHeader.hpp>

namespace xbot::hub {
    class ServiceInterface {
    public:
        explicit ServiceInterface(std::string uid);

        /**
         * Called whenever the service endpoint was changed.
         * This will claim the service on the other sidde and direct its comms to this interface.
         * @return true, on success
         */
        bool OnServiceEndpointChanged();

        bool Start();

    protected:
        bool SendInput(uint16_t id, void* data, size_t size);

    private:

        void SendClaim();
        bool TransmitPacket(const std::vector<uint8_t> &data);

        void RunIo();
        std::atomic_flag stopped_{};
        std::thread io_thread_{};

        // uid is used to lookup the latest endpoint which the service reported.
        // this way we can send messages to the service
        std::string uid_;

        // track, if we have claimed the service successfully.
        // If the service is claimed it will send its outputs to this interface.
        bool claimed_successfully_{false};

        // track when we sent the last claim, so that we don't spam the service
        std::chrono::time_point<std::chrono::steady_clock> last_claim_sent_;

        // Keep a hash of the most important parts of the info around.
        // This way we can verify that an incoming message is actually from the service
        // we expect it to be (theoretically anyone can send messages here and therefore we need to filter).
        // Hashed value = XOR of all hashes: nodeid, serviceid, type, version.
        // If any of those change, our interface rejects the messages from that service.
        size_t service_info_hash_{0};

        // The socket doing the actual communication with the service.
        Socket socket_;

        /**
         * Fetches the relevant info from ServiceDiscovery and updates the local hash.
         *
         * @return true on success
         */
        bool UpdateHash();

        size_t Hash(xbot::comms::datatypes::XbotHeader &header);
    };
};



#endif //SERVICEINTERFACE_HPP
