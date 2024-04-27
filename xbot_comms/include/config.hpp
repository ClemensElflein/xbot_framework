//
// Created by clemens on 3/20/24.
//

#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <cstdint>

namespace xbot::comms::config {
    static constexpr uint16_t max_packet_size = 1500;
    static constexpr uint16_t max_service_count = 25;

    static uint16_t multicast_port = 4242;

    /**
     * Settings for remote logging
     */
    static const char *remote_log_multicast_address = "233.255.255.1";
    static constexpr uint16_t max_log_length = 255;


    /**
     * Settings for Services
     */
    // Max length for the name of a service
    static constexpr uint16_t max_service_name_length = 50;

    /**
     * Settings for Service Discovery
     */
    // Max size in bytes for each service discovery entry.
    // An entry can be a string (service name) or some other data type
    // Make sure to keep this longer as your max_service_name_length+1 (for terminating 0).
    static constexpr uint16_t max_sd_entry_size = 51;
    // Make sure that we have at least space for a 64 bit integer.
    static_assert(max_sd_entry_size > sizeof(uint64_t));
    static_assert(max_sd_entry_size > max_service_name_length);
    static const char *sd_multicast_address = "233.255.255.0";
    static constexpr uint32_t sd_advertisement_interval_micros = 10000000;


    static_assert(max_log_length > 100);
}

#endif //CONFIG_HPP
