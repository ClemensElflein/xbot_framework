cmake_minimum_required(VERSION 3.22)
project(xbot_framework C CXX)


if (XBOT_BUILD_APPS)
    message("Building Apps")
    set(XBOT_BUILD_LIB_SERVICE_INTERFACE ON)
endif ()

if (XBOT_BUILD_EXAMPLES)
    message("Building Examples")
    set(XBOT_BUILD_LIB_SERVICE ON)
endif ()

add_subdirectory(ext)
add_subdirectory(codegen)

if (XBOT_BUILD_LIB_SERVICE)
    message("Building Service Lib")
    add_subdirectory(libxbot-service)
endif ()

if (XBOT_BUILD_LIB_SERVICE_INTERFACE)
    message("Building Service Interface Lib")
    add_subdirectory(libxbot-service-interface)
endif ()

if (XBOT_BUILD_APPS)
    add_subdirectory(tools/xbot_control_hub)
endif ()


if (XBOT_BUILD_EXAMPLES)
    add_subdirectory(example)
    add_subdirectory(example_interface_plugins)
endif ()
