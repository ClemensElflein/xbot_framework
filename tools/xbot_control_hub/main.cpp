#include <Socket.hpp>

#include "crow.h"
#include <xbot/config.hpp>
#include <nlohmann/json.hpp>
#include "ServiceDiscovery.hpp"

using namespace xbot;

int main()
{

    hub::ServiceDiscovery::Start();

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });

    app.port(18080).run();
}