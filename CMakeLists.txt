cmake_minimum_required(VERSION 3.5)
project(lobot_servo)

# Generate compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

# Source
set(lobot_SOURCE src/servo.c)
if(UNIX)
    set(lobot_SOURCE ${lobot_SOURCE} src/port_linux.c)
endif()

add_library(lobot_servo SHARED
    ${lobot_SOURCE}
    )
target_include_directories(lobot_servo PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
install(
  DIRECTORY include/
  DESTINATION include
)
install(
  TARGETS lobot_servo
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_include_directories(
  include
)
ament_export_libraries(
  ${PROJECT_NAME}
)
ament_export_targets(
  export_${PROJECT_NAME}
)

ament_package()

add_subdirectory(utils)
add_subdirectory(examples EXCLUDE_FROM_ALL)
