#include <Socket.hpp>

#include "crow.h"
#include <nlohmann/json.hpp>

#include "CrowToSpeedlogHandler.hpp"
#include "ServiceDiscovery.hpp"
#include "ServiceInterfaceFactory.hpp"

using namespace xbot;


int main() {
    hub::CrowToSpeedlogHandler logger;
    crow::logger::setHandler(&logger);

    // Register the interface factory before starting service discovery
    // this way, whenever a service is found, the appropriate interface is built automatically.
    hub::ServiceDiscovery::RegisterCallbacks(hub::ServiceInterfaceFactory::GetInstance());
    hub::ServiceInterfaceFactory::Start();
    hub::ServiceDiscovery::Start();


    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]() {
        return "Hello world";
    });

    CROW_WEBSOCKET_ROUTE(app, "/socket")
            .onopen([&](crow::websocket::connection &conn) {
                CROW_LOG_INFO << "New Websocket Connection";
            }).onclose([&](crow::websocket::connection &conn, const std::string& reason) {
                CROW_LOG_INFO << "Closed Connection. Reason: " << reason;
            }).onmessage([&](crow::websocket::connection & /*conn*/, const std::string &data, bool is_binary) {
                CROW_LOG_INFO << "New Websocket Message: " << data;
            });

    app.port(18080).run();
}
