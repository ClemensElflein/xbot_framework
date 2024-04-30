//
// Created by clemens on 4/30/24.
//

#include "GenericServiceInterface.hpp"

#include <iostream>

namespace xbot {
namespace hub {
    void GenericServiceInterface::OnOutputDataReceived(uint16_t id, void *data, size_t size) {
        std::cout << "got data!" << std::endl;
    }
} // hub
} // xbot