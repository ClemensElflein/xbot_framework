//
// Created by clemens on 3/25/24.
//

#ifndef LOGPAYLOAD_HPP
#define LOGPAYLOAD_HPP

#include <cstdint>
#include <datatypes/XbotHeader.hpp>
#include <config.hpp>

namespace xbot::comms::datatypes
{
    enum class LogLevel : uint8_t
    {
        UNKNOWN_LEVEL = 0,
        TRACE_LEVEL,
        DEBUG_LEVEL,
        INFO_LEVEL,
        WARNING_LEVEL,
        ERROR_LEVEL,
        CRITICAL_LEVEL,
        ALWAYS_LEVEL,
    };
}


#endif //LOGPAYLOAD_HPP
