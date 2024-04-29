//
// Created by clemens on 3/18/24.
//

#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <xbot/config.hpp>

#include "xbot/datatypes/XbotHeader.hpp"
#include "portable/mutex.hpp"
#include "portable/queue.hpp"
#include "portable/socket.hpp"
#include "portable/thread.hpp"

namespace xbot::comms
{
    class Service
    {
    public:
        explicit Service(uint16_t service_id, uint32_t tick_rate_micros);
        virtual ~Service();
        /*
         * @brief Start the service.
         *
         * This method starts the service.
         *
         * @return True if the service started successfully, false otherwise.
         */
        bool start();


        /**
         * Since the portable thread implementation does not know what a class is, we use this helper to start the service.
         * @param service Pointer to the service to start
         * @return null
         */
        static void* startProcessingHelper(void* service)
        {
            static_cast<Service*>(service)->runProcessing();
            return nullptr;
        }

        /**
         * Since the portable thread implementation does not know what a class is, we use this helper to start the service.
         * @param service Pointer to the service to start
         * @return null
         */
        static void* startIoHelper(void* service)
        {
            static_cast<Service*>(service)->runIo();
            return nullptr;
        }


    protected:
        // Buffer to prepare service advertisements, static to allow reuse between services.
        static uint8_t sd_buffer[config::max_packet_size-sizeof(datatypes::XbotHeader)];
        static MutexPtr sd_buffer_mutex;

        // Scratch space for the header. This will only ever be accessed in the process_thread, so we don't need a mutex
        // Don't make it static, so that multiple services can build packets in parallel (it will happen often)
        datatypes::XbotHeader header_{};

        /**
         * ID of the service, needs to be unique for each node.
         */
        const uint16_t service_id_;
        SocketPtr udp_socket_;

        bool SendData(uint16_t target_id, const void* data, size_t size);
    private:
        /**
         * The main thread for the service.
         * Here the implementation can do its processing.
         */
        ThreadPtr process_thread_ = nullptr;
        /**
         * The IO thread is used to receive and send the data.
         * This allows the process_thread to block as long as it likes.
         */
        ThreadPtr io_thread_ = nullptr;




        MutexPtr state_mutex_;
        QueuePtr packet_queue_ptr_;
        uint32_t tick_rate_micros_;
        uint32_t last_tick_micros_ = 0;
        uint32_t last_service_discovery_micros_ = 0;

        uint32_t target_ip = 0;
        uint32_t target_port = 0;

        bool stopped = true;
        void runProcessing();
        void runIo();

        void fillHeader();

        virtual void advertiseService() = 0;
        virtual void tick() = 0;
        virtual bool handlePacket(const datatypes::XbotHeader* header, const void* payload) = 0;
    };
}

#endif //SERVICE_HPP
