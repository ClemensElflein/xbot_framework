cmake_minimum_required(VERSION 3.25)
project(EchoService C CXX)

# Allow logging using ulog
add_compile_definitions(ULOG_ENABLED)

# Generate the service target
add_service(EchoService ${CMAKE_CURRENT_SOURCE_DIR}/service.json)

add_executable(EchoService main.cpp EchoService.cpp EchoService.hpp)

# Link the generated interface
target_link_libraries(EchoService EchoServiceBase)

