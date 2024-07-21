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
};  // namespace xbot::serviceif

extern "C" const char *PLUGIN_NAME;

/**
 * Called as main entry point for a plugin
 * @param ctx the context to use for service communication
 */
extern "C" void StartPlugin(xbot::serviceif::Context ctx);

/**
 * Called before clean up
 */
extern "C" void StopPlugin();

#endif  // XBOT_FRAMEWORK_PLUGIN_HPP
