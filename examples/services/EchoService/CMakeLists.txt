cmake_minimum_required(VERSION 3.16)
project(EchoService C CXX)


# Generate the service target
add_service(EchoService ${CMAKE_CURRENT_SOURCE_DIR}/service.json)

add_executable(EchoServiceBin main.cpp EchoService.cpp EchoService.hpp)

# Link the generated interface
target_link_libraries(EchoServiceBin EchoService)

# Allow logging using ulog
target_compile_definitions(ulog PUBLIC ULOG_ENABLED)
install(TARGETS EchoServiceBin DESTINATION services)
