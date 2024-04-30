//
// Created by clemens on 4/30/24.
//

#include "ServiceInterfaceFactory.hpp"

#include "GenericServiceInterface.hpp"
#include "spdlog/spdlog.h"
using namespace xbot::hub;

std::map<std::string, std::shared_ptr<ServiceInterfaceBase>> ServiceInterfaceFactory::interfaces_{};
std::shared_ptr<ServiceInterfaceFactory> ServiceInterfaceFactory::instance_ = nullptr;
std::mutex ServiceInterfaceFactory::state_mutex_{};

bool ServiceInterfaceFactory::OnServiceDiscovered(std::string uid) {
    std::unique_lock lk{state_mutex_};
    // Build an interface for this service
    if(interfaces_.contains(uid)) {
        spdlog::error("Interface already exists! (ID: {})", uid);
        return false;
    }

    // Create a ServiceInterface for this uid
    const auto& [i, success] = interfaces_.emplace(uid, std::make_shared<GenericServiceInterface>(uid));
    if(success) {
        i->second->Start();
    }

    return true;
}

bool ServiceInterfaceFactory::OnEndpointChanged(std::string uid) {
    std::unique_lock lk{state_mutex_};
    // Notify the registered interface
    if(interfaces_.contains(uid)) {
        interfaces_.at(uid)->OnServiceEndpointChanged();
    }

    return true;
}

std::shared_ptr<ServiceInterfaceFactory> ServiceInterfaceFactory::GetInstance() {
    std::unique_lock lk{state_mutex_};
    if(instance_ == nullptr) {
        instance_ = std::make_shared<ServiceInterfaceFactory>();
    }
    return instance_;
}
