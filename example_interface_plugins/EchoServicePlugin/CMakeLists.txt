cmake_minimum_required(VERSION 3.22)

project(EchoServicePlugin CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(EchoServicePlugin SHARED main.cpp)

add_service_interface(EchoServiceInterface ${CMAKE_CURRENT_SOURCE_DIR}/../../example/EchoService/service.json)

target_link_libraries(EchoServicePlugin PUBLIC xbot-service-interface EchoServiceInterface)
