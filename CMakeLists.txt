#cmake_minimum_required(VERSION 2.8.3)
cmake_minimum_required(VERSION 3.9.0)
project(five-axis-control-panel)

SET(CMAKE_PREFIX_PATH ${Qt5_DIR})
set(CMAKE_INCLUDE_CURRENT_DIR ON)


## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

find_package(Qt5 COMPONENTS Widgets Core Gui SerialPort Multimedia MultimediaWidgets REQUIRED)
find_package(OpenCV REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS
  ui_description
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${AUTOGEN_BUILD_DIR}
  ${OpenCV_INCLUDE_DIRS}
)

## Declare a C++ library
# add_library(${PROJECT_NAME}
#   src/${PROJECT_NAME}/my_labs.cpp
# )

## Add cmake target dependencies of the library
## as an example, code may need to be generated before libraries
## either from message generation or dynamic reconfigure
# add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Declare a C++ executable
## With catkin_make all packages are built within a single CMake context
## The recommended prefix ensures that target names across packages don't collide
# add_executable(${PROJECT_NAME}_node src/my_labs_node.cpp)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
# set_target_properties(${PROJECT_NAME}_node PROPERTIES OUTPUT_NAME node PREFIX "")

## Add cmake target dependencies of the executable
## same as for the library above
# add_dependencies(${PROJECT_NAME}_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
# target_link_libraries(${PROJECT_NAME}_node
#   ${catkin_LIBRARIES}
# )

add_executable(five-axis-control-panel
  src/main.cpp
  src/main_control_panel.cpp
  src/serial_adapter.cpp
  src/video_adapter.cpp
  src/toggle_switch.cpp
)
# 删掉？add_dependencies(crash_warning_monitor ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(five-axis-control-panel
  ${OpenCV_LIBS}
  Qt5::Core
  Qt5::Gui
  Qt5::Widgets
  Qt5::SerialPort
  Qt5::Multimedia
  Qt5::MultimediaWidgets
)
#qt5_use_modules(${PROJECT_NAME} Core Gui Widgets SerialPort Multimedia MultimediaWidgets)
