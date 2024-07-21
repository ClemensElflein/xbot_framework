cmake_minimum_required(VERSION 3.22)
project(xbot-service-interface C CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(xbot-service-interface
        src/Socket.cpp
        src/ServiceInfo.cpp
        include/xbot-service-interface/ServiceInterfaceBase.hpp
        src/ServiceInterfaceBase.cpp
        include/xbot-service-interface/Plugin.hpp
)

target_include_directories(xbot-service-interface PUBLIC
        include
        ../include
)
target_link_libraries(xbot-service-interface PUBLIC
        nlohmann_json::nlohmann_json spdlog::spdlog
)
set_property(TARGET xbot-service-interface PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET spdlog PROPERTY POSITION_INDEPENDENT_CODE ON)
