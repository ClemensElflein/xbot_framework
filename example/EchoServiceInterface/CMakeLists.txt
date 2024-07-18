cmake_minimum_required(VERSION 3.22)
project(EchoServiceInterface C CXX)

# Generate the service target
add_service_interface(EchoServiceInterface ${CMAKE_CURRENT_SOURCE_DIR}/service.json)

target_sources(EchoServiceInterface PUBLIC EchoServiceInterface.hpp EchoServiceInterface.cpp)
