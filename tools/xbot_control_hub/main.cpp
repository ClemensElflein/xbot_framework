
#include <crow.h>
#include <spdlog/spdlog.h>

#include <EchoServiceInterfaceBase.hpp>

#include "CrowToSpeedlogHandler.hpp"
#include "PlotJugglerBridge.hpp"
#include "ServiceDiscoveryImpl.hpp"
#include "ServiceIoImpl.hpp"
#include "xbot-service-interface/ServiceInterfaceBase.hpp"

using namespace xbot::serviceif;

std::mutex m;
std::condition_variable cond_var;
std::string last_echo{};

int timeout = 0;
int error = 0;
int ok = 0;

class EchoServiceInterface : public EchoServiceInterfaceBase {
 public:
  explicit EchoServiceInterface(uint16_t service_id, Context ctx)
      : EchoServiceInterfaceBase(service_id, ctx) {}

 protected:
 public:
  bool OnConfigurationRequested(const std::string &uid) override {
    spdlog::info("Config Requested");
    std::string prefix = "Some Prefix";
    StartTransaction(true);
    SetRegisterPrefix(prefix.c_str(), prefix.length());
    SetRegisterEchoCount(2);
    CommitTransaction();
    return true;
  }

 protected:
  void OnEchoChanged(const char *new_value, uint32_t length) override {
    std::string e = std::string(new_value, length);
    spdlog::info("Got echo {}", e);
    if (e.starts_with("this is a test message ")) {
      std::lock_guard<std::mutex> lock(m);
      last_echo = e;
      cond_var.notify_one();
    }
  }
  void OnMessageCountChanged(const uint32_t &new_value) override {
    // spdlog::info("Got count {}", new_value);
  }
};

void echoThread() {
  const Context ctx{.io = ServiceIOImpl::GetInstance(),
                    .serviceDiscovery = ServiceDiscoveryImpl::GetInstance()};
  EchoServiceInterface si{1, ctx};
  si.Start();
  int i = 0;
  while (1) {
    std::unique_lock<std::mutex> lock(m);
    std::string str =
        std::string("this is a test message ") + std::to_string(i++);
    // spdlog::info("-- sending");
    auto start = std::chrono::steady_clock::now();
    si.SendInputText(str.c_str(), str.length());
    const auto stat = cond_var.wait_for(lock, std::chrono::milliseconds(10));

    if (stat == std::cv_status::no_timeout) {
      // spdlog::info("-- got echo");
      // got data
      if (last_echo == str) {
        auto end = std::chrono::steady_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start)
                .count();
        spdlog::info("ping: {} uS", duration);
        ok++;
      } else {
        error++;
      }
    } else {
      // spdlog::info("-- got TO");
      timeout++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if ((ok + error + timeout) % 100 == 0) {
      spdlog::info("error: {}", error);
      spdlog::info("timeout: {}", timeout);
      spdlog::info("ok: {}", ok);
    }
  }
}

int main() {
  hub::CrowToSpeedlogHandler logger;
  crow::logger::setHandler(&logger);

  const auto ioImpl = ServiceIOImpl::GetInstance();
  const auto sdImpl = ServiceDiscoveryImpl::GetInstance();

  const Context ctx{.io = ioImpl, .serviceDiscovery = sdImpl};

  // Register the ServiceIO before starting service discovery
  // this way, whenever a service is found, ServiceIO claims it automatically
  ctx.serviceDiscovery->RegisterCallbacks(ioImpl);

  PlotJugglerBridge pjb{ctx};

  ioImpl->Start();
  sdImpl->Start();

  crow::SimpleApp app;

  std::thread echo{echoThread};

  int i = 0;
  CROW_ROUTE(app, "/")
  ([]() { return "Hello world"; });
  CROW_ROUTE(app, "/services")
  ([sdImpl]() {
    nlohmann::json result = nlohmann::detail::value_t::object;
    const auto services = sdImpl->GetAllServices();

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
