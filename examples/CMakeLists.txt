cmake_minimum_required(VERSION 3.5)
project(lobot_example)

add_executable(simple simple.c)
target_link_libraries(simple PUBLIC lobot_servo)
target_include_directories(simple PUBLIC
    "${PROJECT_BINARY_DIR}"
    )
