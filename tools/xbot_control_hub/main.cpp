#include <Socket.hpp>

#include "crow.h"
#include <nlohmann/json.hpp>

#include "CrowToSpeedlogHandler.hpp"
#include "ServiceDiscovery.hpp"
#include "ServiceInterfaceFactory.hpp"

using namespace xbot;


int main()
{
    hub::CrowToSpeedlogHandler logger;
    crow::logger::setHandler(&logger);

    // Register the interface factory before starting service discovery
    // this way, whenever a service is found, the appropriate interface is built automatically.
    hub::ServiceDiscovery::RegisterCallbacks(hub::ServiceInterfaceFactory::GetInstance());
    hub::ServiceDiscovery::Start();




    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    app.port(18080).run();
}