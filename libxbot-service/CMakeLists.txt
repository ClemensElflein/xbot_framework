cmake_minimum_required(VERSION 3.25)
project(libxbot-service C CXX)


#set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Allow logging using ulog
#add_compile_definitions(ULOG_ENABLED)

# Add external dependencies
add_subdirectory(ext)

add_library(libxbot-service STATIC
        src/Service.cpp
        src/Lock.cpp
        src/RemoteLogging.cpp
        src/ServiceIo.cpp
)

if (NOT DEFINED XBOT_CUSTOM_PORT_PATH)
    SET(XBOT_CUSTOM_PORT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/portable/linux)
else ()
    message("Using custom port at ${XBOT_CUSTOM_PORT_PATH}")
endif ()

file(GLOB_RECURSE PORT_SOURCES ${XBOT_CUSTOM_PORT_PATH}/*.c ${XBOT_CUSTOM_PORT_PATH}/*.cpp ${XBOT_CUSTOM_PORT_PATH}/**/*.c ${XBOT_CUSTOM_PORT_PATH}/**/*.cpp)
target_include_directories(libxbot-service PUBLIC ${XBOT_CUSTOM_PORT_PATH}/include)
target_sources(libxbot-service PRIVATE ${PORT_SOURCES})


target_include_directories(libxbot-service PUBLIC include ${PROJECT_SOURCE_DIR}/../include)
target_link_libraries(libxbot-service PUBLIC ulog etl)
#set_property(TARGET libxbot-service PROPERTY CXX_STANDARD 23)

#add_subdirectory(test)
