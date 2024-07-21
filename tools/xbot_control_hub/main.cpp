
#include <crow.h>
#include <dlfcn.h>
#include <spdlog/spdlog.h>

#include <filesystem>

#include "CrowToSpeedlogHandler.hpp"
#include "PlotJugglerBridge.hpp"
#include "ServiceDiscoveryImpl.hpp"
#include "ServiceIoImpl.hpp"

using namespace xbot::serviceif;
namespace fs = std::filesystem;

void loadPlugins(const fs::path &path, Context ctx) {
  if (!fs::exists(path) || !fs::is_directory(path)) {
    std::cout << "Directory " << path << " does not exist";
    return;
  }

  for (const auto &entry : fs::directory_iterator(path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".so") {
      void *handle = dlopen(entry.path().c_str(), RTLD_NOW);
      if (handle) {
        const auto pluginName = (const char **)dlsym(handle, "PLUGIN_NAME");
        {
          const char *dlsym_error = dlerror();
          if (dlsym_error) {
            std::cerr << "Could not load 'entry': " << dlsym_error << '\n';
            dlclose(handle);
            continue;
          }
        }
        std::cout << "found plugin " << *pluginName << std::endl;
        const auto entryFunction =
            (void (*)(Context))dlsym(handle, "StartPlugin");
        const char *dlsym_error = dlerror();
        if (dlsym_error) {
          std::cerr << "Could not load 'entry': " << dlsym_error << '\n';
          dlclose(handle);
        } else {
          if (entryFunction) entryFunction(ctx);
          dlclose(handle);
        }
      } else {
        std::cerr << "Could not open library: " << dlerror() << '\n';
      }
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

  std::string plugin_dirs = std::getenv("XBOT_PLUGIN_DIRS");

  loadPlugins(
      "/home/clemens/Dev/xbot_framework/xbot_framework/build/Debug/"
      "example_interface_plugins/"
      "EchoServicePlugin",
      ctx);

  ioImpl->Start();
  sdImpl->Start();

  crow::SimpleApp app;

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
