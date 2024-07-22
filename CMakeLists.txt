cmake_minimum_required(VERSION 3.22)
project(xbot_framework C CXX)

set(CPACK_COMPONENTS_ALL "")


# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif ()


if (XBOT_BUILD_RUNTIME)
    message("Building Runtime")
    set(XBOT_BUILD_LIB_SERVICE_INTERFACE ON)
endif ()

if (XBOT_BUILD_EXAMPLES)
    message("Building Examples")
    set(XBOT_BUILD_LIB_SERVICE ON)
endif ()

add_subdirectory(ext EXCLUDE_FROM_ALL)
add_subdirectory(codegen EXCLUDE_FROM_ALL)

if (XBOT_BUILD_LIB_SERVICE)
    message("Building Service Lib")
    add_subdirectory(libxbot-service)
    list(APPEND CPACK_COMPONENTS_ALL xbot-service)
endif ()

if (XBOT_BUILD_LIB_SERVICE_INTERFACE)
    message("Building Service Interface Lib")
    add_subdirectory(libxbot-service-interface)
    list(APPEND CPACK_COMPONENTS_ALL xbot-service-interface)
endif ()

if (XBOT_BUILD_EXAMPLES)
    add_subdirectory(examples)
    list(APPEND CPACK_COMPONENTS_ALL EchoServiceBin EchoServiceExample)
endif ()
include(CPack)
