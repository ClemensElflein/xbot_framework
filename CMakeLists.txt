cmake_minimum_required(VERSION 3.25)
project(xbot_framework C CXX)

add_subdirectory(ext)
add_subdirectory(codegen)
add_subdirectory(libxbot-service)
add_subdirectory(libxbot-service-interface)

if (${XBOT_BUILD_APPS})
    message("Building Apps")
    add_subdirectory(tools/xbot_control_hub)
endif ()

if (${XBOT_BUILD_EXAMPLES})
    message("Building Examples")
    add_subdirectory(example)
endif ()
