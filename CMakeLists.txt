cmake_minimum_required(VERSION 3.25)
project(xbot_framework C CXX)

add_subdirectory(codegen)
add_subdirectory(libxbot-service)

if (${XBOT_BUILD_APPS})
    message("Building Apps")
    add_subdirectory(tools/xbot_control_hub)
endif ()

if (${XBOT_BUILD_EXAMPLES})
    message("Building Examples")
    add_subdirectory(example)
endif ()
