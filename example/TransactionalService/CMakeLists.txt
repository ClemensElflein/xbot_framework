cmake_minimum_required(VERSION 3.22)
project(TransactionalService C CXX)


# Generate the service target
add_service(TransactionalService ${CMAKE_CURRENT_SOURCE_DIR}/service.json)

add_executable(TransactionalServiceBin main.cpp TransactionalService.cpp TransactionalService.hpp)

# Link the generated interface
target_link_libraries(TransactionalServiceBin TransactionalService ${MESSAGE_LIBS})

# Allow logging using ulog
target_compile_definitions(ulog PUBLIC ULOG_ENABLED)
