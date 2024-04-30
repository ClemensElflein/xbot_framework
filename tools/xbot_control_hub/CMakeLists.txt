cmake_minimum_required(VERSION 3.25)
project(xbot_hub C CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(ext)

add_executable(xbot_hub main.cpp Socket.cpp
        ServiceDiscovery.cpp
        ServiceDiscovery.hpp
        ServiceInfo.hpp
        ServiceInputInfo.hpp
        endpoint_utils.hpp
        ServiceInterface.cpp
        ServiceInterface.hpp
        ServiceInterface.cpp
        ServiceInterface.hpp
        ServiceInfo.cpp
        CrowToSpeedlogHandler.hpp
        ServiceInterfaceFactory.cpp
        ServiceInterfaceFactory.hpp)
target_include_directories(xbot_hub PRIVATE . ${PROJECT_SOURCE_DIR}/../../include)
target_link_libraries(xbot_hub PRIVATE nlohmann_json::nlohmann_json cli::cli Crow spdlog::spdlog)