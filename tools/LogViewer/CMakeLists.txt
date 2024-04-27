add_executable(LogViewer
        src/app.cpp
        src/app.hpp
)

target_link_libraries(LogViewer PRIVATE xbot_comms)