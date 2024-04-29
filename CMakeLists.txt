cmake_minimum_required(VERSION 3.25)
project(xbot_framework C CXX)

add_subdirectory(codegen)
add_subdirectory(xbot_comms)

add_subdirectory(tools/xbot_control_hub)
add_subdirectory(example)