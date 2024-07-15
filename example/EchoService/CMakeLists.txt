cmake_minimum_required(VERSION 3.25)
project(EchoService C CXX)


# Generate the service target
add_service(EchoService ${CMAKE_CURRENT_SOURCE_DIR}/service.json)

add_message(Pet ${CMAKE_CURRENT_SOURCE_DIR}/pet.cddl)

set(MESSAGE_LIBS ZCBOR_Pet)

target_link_libraries(EchoService PUBLIC ${MESSAGE_LIBS})

add_executable(EchoServiceBin main.cpp EchoService.cpp EchoService.hpp)

# Link the generated interface
target_link_libraries(EchoServiceBin EchoService ${MESSAGE_LIBS})

# Allow logging using ulog
target_compile_definitions(ulog PUBLIC ULOG_ENABLED)
