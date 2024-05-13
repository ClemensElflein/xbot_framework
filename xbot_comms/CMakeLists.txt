cmake_minimum_required(VERSION 3.25)
project(xbot_comms C CXX)


#set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Allow logging using ulog
add_compile_definitions(ULOG_ENABLED)

# Add external dependencies
add_subdirectory(ext)

add_library(xbot_comms STATIC
        src/Service.cpp
        src/Lock.cpp
        src/RemoteLogging.cpp
)

if (${XBOT_ADD_LINUX_PORTABLE})
    target_sources(xbot_comms
            src/portable/linux/mutex.cpp
            src/portable/linux/thread.cpp
            src/portable/linux/socket.cpp
            src/portable/linux/packet.cpp
            src/portable/linux/queue.cpp
            src/portable/linux/system.cpp
    )
    target_include_directories(xbot_comms PUBLIC src/portable/linux/include)
endif ()

target_include_directories(xbot_comms PUBLIC include ${PROJECT_SOURCE_DIR}/../include)
target_link_libraries(xbot_comms PUBLIC ulog etl)
#set_property(TARGET xbot_comms PROPERTY CXX_STANDARD 23)

add_subdirectory(test)
