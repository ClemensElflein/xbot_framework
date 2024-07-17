//
// Created by clemens on 7/16/24.
//

#ifndef SERVICEIOINFO_HPP
#define SERVICEIOINFO_HPP

struct ServiceIOInfo {
  // Id of this input or output (used to send and receive data)
  uint16_t id{};

  // Name of this input or output e.g. "speed"
  std::string name{};

  // Type e.g. "uint32_t" or "char[100]"
  std::string type{};

  // Optional string specifying the encoding (e.g. zcbor, or raw if none is
  // given)
  std::string encoding{};
};

inline void to_json(nlohmann::json &nlohmann_json_j,
                    const ServiceIOInfo &nlohmann_json_t) {
  nlohmann_json_j["id"] = nlohmann_json_t.id;
  nlohmann_json_j["name"] = nlohmann_json_t.name;
  nlohmann_json_j["type"] = nlohmann_json_t.type;
  if (!nlohmann_json_t.encoding.empty()) {
    nlohmann_json_j["encoding"] = nlohmann_json_t.encoding;
  }
}

inline void from_json(const nlohmann::json &nlohmann_json_j,
                      ServiceIOInfo &nlohmann_json_t) {
  nlohmann_json_j.at("id").get_to(nlohmann_json_t.id);
  nlohmann_json_j.at("name").get_to(nlohmann_json_t.name);
  nlohmann_json_j.at("type").get_to(nlohmann_json_t.type);
  if (nlohmann_json_j.contains("encoding")) {
    nlohmann_json_j.at("encoding").get_to(nlohmann_json_t.encoding);
  }
}
#endif  // SERVICEIOINFO_HPP
