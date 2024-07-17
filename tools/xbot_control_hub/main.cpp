
#include <crow.h>
#include <spdlog/spdlog.h>

#include "CrowToSpeedlogHandler.hpp"
#include "PlotJugglerBridge.hpp"

using namespace xbot::serviceif;

int main() {
  hub::CrowToSpeedlogHandler logger;
  crow::logger::setHandler(&logger);

  // Register the ServiceIO before starting service discovery
  // this way, whenever a service is found, ServiceIO claims it automatically
  ServiceDiscovery::RegisterCallbacks(ServiceIO::GetInstance());

  PlotJugglerBridge pjb;

  ServiceIO::Start();
  ServiceDiscovery::Start();

  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([]() { return "Hello world"; });
  CROW_ROUTE(app, "/services")
  ([]() {
    nlohmann::json result = nlohmann::detail::value_t::object;
    const auto services = ServiceDiscovery::GetAllSerivces();

    for (const auto &s : *services) {
      result[s.first] = s.second;
    }
    return result.dump(2);
  });

  CROW_WEBSOCKET_ROUTE(app, "/socket")
      .onopen([&](crow::websocket::connection &conn) {
        CROW_LOG_INFO << "New Websocket Connection";
      })
      .onclose(
          [&](crow::websocket::connection &conn, const std::string &reason) {
            CROW_LOG_INFO << "Closed Connection. Reason: " << reason;
          })
      .onmessage([&](crow::websocket::connection & /*conn*/,
                     const std::string &data, bool is_binary) {
        CROW_LOG_INFO << "New Websocket Message: " << data;
      });

  app.port(18080).run();
}
