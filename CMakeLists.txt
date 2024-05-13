cmake_minimum_required(VERSION 3.25)
project(xbot_framework C CXX)

add_subdirectory(codegen)
add_subdirectory(xbot_comms)

if (${XBOT_BUILD_APPS})
    add_subdirectory(tools/xbot_control_hub)
endif ()

if (${XBOT_BUILD_EXAMPLES})
    add_subdirectory(example)
endif ()
