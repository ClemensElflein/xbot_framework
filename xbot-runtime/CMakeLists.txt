cmake_minimum_required(VERSION 3.22)
project(xbot-runtime C CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(xbot-runtime
        main.cpp
        ServiceDiscoveryImpl.cpp
        ServiceIO.cpp
        PlotJugglerBridge.cpp
        ServiceIoImpl.hpp)
target_link_libraries(xbot-runtime PRIVATE cli::cli cli::cli Crow xbot-service-interface EchoServiceInterface)
