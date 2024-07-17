//
// Created by clemens on 7/16/24.
//

#include "PlotJugglerBridge.hpp"

#include "spdlog/spdlog.h"

using namespace xbot::serviceif;

PlotJugglerBridge::PlotJugglerBridge() {
  socket_.Start();
  ServiceDiscovery::RegisterCallbacks(this);
}
PlotJugglerBridge::~PlotJugglerBridge() {
  ServiceIO::UnregisterCallbacks(this);
}
bool PlotJugglerBridge::OnServiceDiscovered(std::string uid) {
  // Query the service description and add build the map
  std::unique_lock lk{state_mutex_};

  const auto info = ServiceDiscovery::GetServiceInfo(uid);
  for (const auto& output : info->description.outputs) {
    topic_map_[std::make_pair(uid, output.id)] = output;
  }

  // We are interested in all discovered services, so we register us with the
  // ServiceIO as soon as a new service is discovered
  ServiceIO::RegisterCallbacks(uid, this);
  return true;
}
bool PlotJugglerBridge::OnEndpointChanged(std::string uid, uint32_t old_ip,
                                          uint16_t old_port, uint32_t new_ip,
                                          uint16_t new_port) {
  // We don't care, ServiceIO will handle this for us.
  return false;
}

void PlotJugglerBridge::OnServiceConnected(const std::string& uid) {
  spdlog::info("PJB: Service Connected!");
}
void PlotJugglerBridge::OnData(const std::string& uid,
                               const datatypes::XbotHeader& header,
                               const void* payload) {
  std::unique_lock lk{state_mutex_};
  // Find the output description
  const auto& it = topic_map_.find(std::make_pair(uid, header.arg2));
  if (it == topic_map_.end()) {
    spdlog::warn("PJB: Data packet with invalid ID");
    return;
  }

  const auto& output = it->second;

  if (output.encoding != "zcbor") return;

  // Try parse the CBOR and send it
  try {
    // Build the JSON and send it
    nlohmann::json json_payload =
        nlohmann::json::from_cbor(static_cast<const uint8_t*>(payload));
    spdlog::info(json_payload.dump());
    nlohmann::json json =
        nlohmann::json::object({{uid,
                                 {{output.name,
                                   {{"sequence", header.sequence_no},
                                    {"stamp", header.timestamp},
                                    {"data", json_payload}}}}}});
    std::string dump = json.dump(2);
    socket_.TransmitPacket("127.0.0.1", 9870,
                           reinterpret_cast<const uint8_t*>(dump.c_str()),
                           dump.size());
  } catch (std::exception& e) {
    spdlog::warn("Exception parsing CBOR data: {}", e.what());
  }
}
void PlotJugglerBridge::OnServiceDisconnected(const std::string& uid) {
  spdlog::info("PJB: OnServiceDisconnected");
}
