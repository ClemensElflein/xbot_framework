
#include <crow.h>
#include <spdlog/spdlog.h>

#include "CrowToSpeedlogHandler.hpp"
#include "PlotJugglerBridge.hpp"
#include "xbot-service-interface/ServiceInterfaceBase.hpp"

using namespace xbot::serviceif;

class EchoServiceInterface : public ServiceInterfaceBase {
 public:
  explicit EchoServiceInterface(uint16_t service_id)
      : ServiceInterfaceBase(service_id, "EchoService", 1) {}

  int i = 0;
  void SendSomething() {
    StartTransaction();
    i++;
    std::string text = "this is a test " + std::to_string(i);
    SendData(0, text.c_str(), text.length());
    uint32_t j = i;
    SendData(1, &j, sizeof(j));
    CommitTransaction();
  }
  void OnServiceConnected(const std::string &uid) override {
    spdlog::info("service connected");
  };
  void OnTransactionStart(uint64_t timestamp) override{};
  void OnTransactionEnd() override{};
  void OnData(const std::string &uid, uint64_t timestamp, uint16_t target_id,
              const void *payload, size_t buflen) override {
    spdlog::info("ondata");
  }
  void OnServiceDisconnected(const std::string &uid) override {
    spdlog::info("service disconnected");
  };
};

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

  EchoServiceInterface si{1};
  si.Start();

  CROW_ROUTE(app, "/")
  ([&si]() {
    si.SendSomething();
    return "Hello world";
  });
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
