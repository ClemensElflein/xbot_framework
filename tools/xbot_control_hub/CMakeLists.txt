cmake_minimum_required(VERSION 3.25)
project(xbot_hub C CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(xbot_hub
        main.cpp
        PlotJugglerBridge.cpp)
target_link_libraries(xbot_hub PRIVATE cli::cli cli::cli Crow xbot-service-interface)
