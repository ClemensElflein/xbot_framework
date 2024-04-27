add_executable(Example
        src/app.cpp
)

target_link_libraries(Example PRIVATE xbot_comms)