cmake_minimum_required(VERSION 3.5)
project(lobot_util)

if(UNIX)
add_executable(lobot_util lobot_util.c)
target_link_libraries(lobot_util PUBLIC lobot_servo)
target_include_directories(lobot_util PUBLIC
     "${PROJECT_BINARY_DIR}"
     )
endif()
