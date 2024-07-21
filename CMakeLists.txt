cmake_minimum_required(VERSION 3.22)
project(xbot_framework C CXX)

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

if (XBOT_BUILD_RUNTIME)
    add_subdirectory(xbot-runtime)
endif ()


if (XBOT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()
