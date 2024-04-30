//
// Created by clemens on 4/30/24.
//

#ifndef SERVICEINTERFACE_HPP
#define SERVICEINTERFACE_HPP

#include <string>
#include <cstddef>
#include <Socket.hpp>
#include <thread>

namespace xbot::hub {
    class ServiceInterfaceBase {
    public:
        virtual ~ServiceInterfaceBase() = default;
        explicit ServiceInterfaceBase(std::string uid);

        /**
         * Called whenever the service endpoint was changed.
         * This will claim the service on the other side and direct its comms to this interface.
         * @return true, on success
         */
        bool OnServiceEndpointChanged();

        bool Start();

    protected:
        //bool SendInput(uint16_t id, void* data, size_t size);

        virtual void OnOutputDataReceived(uint16_t id, void* data, size_t size) = 0;

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
        std::atomic_flag claimed_successfully_{false};
        uint32_t service_ip_{0};
        uint16_t service_port_{0};

        // track when we sent the last claim, so that we don't spam the service
        std::chrono::time_point<std::chrono::steady_clock> last_claim_sent_{std::chrono::seconds(0)};
        std::chrono::time_point<std::chrono::steady_clock> last_heartbeat_received_{std::chrono::seconds(0)};


        // The socket doing the actual communication with the service.
        Socket socket_;
    };
};



#endif //SERVICEINTERFACE_HPP
