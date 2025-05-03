cmake_minimum_required(VERSION 3.0.2)
project(multi_map_navigation)

# Find required ROS packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  actionlib
  actionlib_msgs
  move_base_msgs
  geometry_msgs
  tf
  std_msgs
  message_generation
)

# Find SQLite3 for database handling
find_package(SQLite3 REQUIRED)

# Specify action files to generate messages
add_action_files(
  DIRECTORY action
  FILES MultiMapNavigation.action
)

# Generate messages with dependencies
generate_messages(
  DEPENDENCIES
  actionlib_msgs
  geometry_msgs
  std_msgs
)

# Declare catkin package
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS
    roscpp
    actionlib
    actionlib_msgs
    move_base_msgs
    geometry_msgs
    tf
    std_msgs
    message_runtime
  DEPENDS SQLite3
)

# Specify include directories
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${SQLite3_INCLUDE_DIRS}
)

# Build the library
add_library(${PROJECT_NAME}
  src/multi_map_navigator.cpp
)
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${PROJECT_NAME}
  ${catkin_LIBRARIES}
  ${SQLite3_LIBRARIES}
)

# Build the executable
add_executable(multi_map_navigation_node src/multi_map_navigation_node.cpp)
add_dependencies(multi_map_navigation_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(multi_map_navigation_node
  ${catkin_LIBRARIES}
  ${SQLite3_LIBRARIES}
  ${PROJECT_NAME}
)
