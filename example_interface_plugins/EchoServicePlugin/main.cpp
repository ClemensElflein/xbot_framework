//
// Created by clemens on 7/21/24.
//
#include <spdlog/spdlog.h>

#include <EchoServiceInterfaceBase.hpp>
#include <xbot-service-interface/Plugin.hpp>

using namespace xbot::serviceif;
std::mutex m;
std::condition_variable cond_var;
std::string last_echo{};

int timeout = 0;
int error_count = 0;
int ok = 0;

class EchoServiceInterface : public EchoServiceInterfaceBase {
 public:
  explicit EchoServiceInterface(uint16_t service_id, Context ctx)
      : EchoServiceInterfaceBase(service_id, ctx) {}

  ~EchoServiceInterface() override {
    spdlog::error("Deconstructing interface");
  }

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

Context ctx{};

void echoThread() {
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
        error_count++;
      }
    } else {
      // spdlog::info("-- got TO");
      timeout++;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if ((ok + error_count + timeout) % 100 == 0) {
      spdlog::info("error_count: {}", error_count);
      spdlog::info("timeout: {}", timeout);
      spdlog::info("ok: {}", ok);
    }
  }
}

std::unique_ptr<std::thread> echo_thread = nullptr;

void StartPlugin(Context c) {
  spdlog::info("Got Start Plugin call2!");
  ctx = c;
  echo_thread = std::move(std::make_unique<std::thread>(echoThread));
}
void StopPlugin() { spdlog::info("Got Stop Plugin call!"); }

const char *PLUGIN_NAME = "EchoServicePlugin";
