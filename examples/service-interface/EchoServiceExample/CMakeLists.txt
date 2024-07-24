cmake_minimum_required(VERSION 3.16)

project(EchoServiceExample CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the plugin as shared libary
add_executable(EchoServiceExample main.cpp EchoServiceInterface.cpp)

# Generate the code for the EchoService
add_service_interface(EchoServiceInterface ${CMAKE_CURRENT_SOURCE_DIR}/../../services/EchoService/service.json)

# Add the interface to the plugin
target_link_libraries(EchoServiceExample PRIVATE EchoServiceInterface)
install(TARGETS EchoServiceExample DESTINATION service-interface)
