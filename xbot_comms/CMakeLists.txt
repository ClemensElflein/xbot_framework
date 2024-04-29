cmake_minimum_required(VERSION 3.25)
project(xbot_comms C CXX)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Allow logging using ulog
add_compile_definitions(ULOG_ENABLED)

# Add external dependencies
add_subdirectory(ext)

add_library(xbot_comms STATIC
        include/Service.hpp
        include/portable/queue.hpp
        include/portable/socket.hpp
        include/portable/packet.hpp
        include/portable/mutex.hpp
        include/portable/thread.hpp
        src/Service.cpp
        src/portable/linux/mutex.cpp
        src/portable/linux/thread.cpp
        src/portable/linux/socket.cpp
        src/portable/linux/packet.cpp
        src/portable/linux/queue.cpp
        include/Lock.hpp
        src/Lock.cpp
        include/portable/system.hpp
        src/portable/linux/system.cpp
        include/RemoteLogging.hpp
        src/RemoteLogging.cpp
)

target_include_directories(xbot_comms PUBLIC include ${PROJECT_SOURCE_DIR}/../include)
target_link_libraries(xbot_comms PUBLIC ulog etl)
set_property(TARGET xbot_comms PROPERTY CXX_STANDARD 23)

add_subdirectory(test)
