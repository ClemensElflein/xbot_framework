//
// Created by clemens on 7/21/24.
//
#include <spdlog/spdlog.h>

#include <xbot-service-interface/Plugin.hpp>

#include "EchoServiceInterface.hpp"

using namespace xbot::serviceif;

Context ctx{};

std::atomic_flag stopped{false};
std::thread* echo_thread = nullptr;

void echoThread() {
  EchoServiceInterface si{1, ctx};
  si.Start();
  int i = 0;
  while (!stopped.test()) {
    std::string input_text = "Echo Request " + std::to_string(i++);
    // Send a message to the service. The echo will be printed by the
    // EchoServiceInterface implementation.
    si.SendInputText(input_text.c_str(), input_text.length());
    // Wait a second.
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
}

/**
 * Plugin entry point. This will be called by xbot_framework after discovering
 * the plugin.
 * @param c the context to use for io and service discovery. Just hand it to the
 * generated code.
 */
void StartPlugin(Context c) {
  spdlog::info("Got Start Plugin call.");
  // Store the context
  ctx = c;
  // Create a thread to send echos to EchoService
  echo_thread = new std::thread(echoThread);
}
void StopPlugin() {
  spdlog::info("Got Stop Plugin call, shutting down thread.");
  // Set the stopped flag and wait for the thread to terminate
  stopped.test_and_set();
  echo_thread->join();
}

const char* PLUGIN_NAME = "EchoServicePlugin";
