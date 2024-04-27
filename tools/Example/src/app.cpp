//
// Created by clemens on 3/21/24.
//
#include <condition_variable>
#include <csignal>

#include <iostream>
#include <thread>
#include "Service.hpp"
#include <semaphore>
#include <ulog.h>

#include "RemoteLogging.hpp"
#include "../../../xbot_comms/src/InputChannel.hpp"
#include "portable/system.hpp"


void console_logger(ulog_level_t severity, char *msg, void* arg) {
    printf("[%s]: %s\n",
        ulog_level_name(severity),
        msg);
}

class MyService : public xbot::comms::Service
{
protected:


private:
    bool handlePacket(const xbot::comms::datatypes::XbotHeader *header, const void *payload) override {
        return true;
    }

    void tick() override
    {
        ULOG_WARNING("tick");
    }


public:
    explicit consteval MyService()
        : Service(1, 1000000)
    {

    }

    //xbot::comms::InputChannel<float> my_prop{1, "asdf"};
    //xbot::comms::InputChannel<float> my_prop1{0, "asdf"};
    // xbot::comms::InputChannel<uint32_t> my_prop{1, ""};
    // xbot::comms::InputChannel<uint8_t> my_prop2{2, ""};
    const xbot::comms::InputChannel<uint8_t> my_prop1{0, "asdfadf"};
    const xbot::comms::InputChannel<uint8_t> my_prop2{2, ""};
    const xbot::comms::InputChannel<uint8_t> my_prop3{2, "asdf"};


    const etl::flat_map<uint16_t, const xbot::comms::PacketHandler*, 3> handler_map {
        etl::pair(0, &my_prop1),
        etl::pair(0, &my_prop1),
        etl::pair(0, &my_prop1),
        etl::pair(0, &my_prop1),
        etl::pair(0, &my_prop1),
        etl::pair(0, &my_prop1),
        etl::pair(0, &my_prop1)
    };

    // const xbot::comms::PacketHandler* handlers[] = {&my_prop1,&my_prop2,&my_prop3};

    // bool handlePacket(const xbot::comms::datatypes::XbotHeader* header, const void* payload) override
    // {
    //     std::cout << "got packet" << std::endl;
    //     return true;
    // }

public:
    ~MyService() override = default;
};


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

void* logThread(void* args)
{
    while(1)
    {
        sleep(1);
        ULOG_WARNING("test log 1");
        uint16_t service_id = 42;
        ULOG_ARG_WARNING(&service_id, "test log 2");
    }
}

template <uint32_t V>
struct force_compilation
{
    static constexpr uint32_t value = V;
};

consteval size_t test() {
    constexpr xbot::comms::InputChannel<uint8_t> my_prop1{0, "asdfadf"};
    constexpr uint8_t test[100] = {};
    constexpr auto description = my_prop1.describeChannel();
    return description.data_type_length;
}

int main()
{



    initSystem();
    ULOG_SUBSCRIBE(console_logger, ULOG_DEBUG_LEVEL);
    // xbot::comms::IO io{};
    startRemoteLogging();
    // io.start();
    MyService service{};

    constexpr auto asdf = test();
    static_assert(asdf == 42, "serialization done");

    service.start();
    // io.registerService(&service);

    xbot::comms::createThread(logThread, nullptr);


    // Set up signal handling function for SIGINT (CTRL+C)
    std::signal(SIGINT, handleSignal);
    // Wait for signal
    sema.acquire();

    return 0;
}
