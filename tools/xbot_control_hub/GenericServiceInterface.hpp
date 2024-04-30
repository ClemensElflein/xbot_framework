//
// Created by clemens on 4/30/24.
//

#ifndef GENERICSERVICEINTERFACE_HPP
#define GENERICSERVICEINTERFACE_HPP
#include "ServiceInterfaceBase.hpp"

namespace xbot::hub {

/**
* This service interface gets instanciated if no specialized interface can be found for this service.
*/
class GenericServiceInterface : public ServiceInterfaceBase {
public:
    explicit GenericServiceInterface(std::string uid)
        : ServiceInterfaceBase(std::move(uid)) {
    }
    ~GenericServiceInterface() override = default;

protected:
    void OnOutputDataReceived(uint16_t id, void *data, size_t size) override;

};

}
#endif //GENERICSERVICEINTERFACE_HPP
