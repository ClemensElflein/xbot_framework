//
// Created by clemens on 7/21/24.
//

#ifndef XBOT_FRAMEWORK_PLUGIN_HPP
#define XBOT_FRAMEWORK_PLUGIN_HPP

#include <xbot-service-interface/ServiceDiscovery.hpp>
#include <xbot-service-interface/ServiceIO.hpp>

namespace xbot::serviceif {
struct Context {
  ServiceIO *io = nullptr;
  ServiceDiscovery *serviceDiscovery = nullptr;
};
}  // namespace xbot::serviceif

#endif  // XBOT_FRAMEWORK_PLUGIN_HPP
