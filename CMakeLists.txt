cmake_minimum_required(VERSION 2.8.3)
project(tradr_uav_fpv)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
    roscpp
    bridge_path_interface_msgs
    uav_videostream_msgs
)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS system)

find_package(Qt4 REQUIRED)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules
)

find_package(libvlc REQUIRED)
find_package(Exiv2 REQUIRED)

set(LIBVLC_LIBRARIES
    ${LIBVLCCORE_LIBRARY}
    ${LIBVLC_LIBRARY}
)

catkin_package()


# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_AUTOMOC OFF)
# set(QT_USE_QTGUI TRUE)




###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${QT_INCLUDE_DIRS}
  ${LIBVLC_INCLUDE_DIRS}
)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#link_directories(${catkin_LIBRARY_DIRS})



file(GLOB_RECURSE SRC_FILES
    "*.c"
    "*.cpp"
    "*.cc"
    "*.cxx"
    "*.h"
    "*.hpp"
    "*.hh"
    "*.hxx"
)

foreach(f ${SRC_FILES})
    if(NOT f MATCHES "main.c")
        list(APPEND SRC_FILES_TO_WARP ${f})
    endif()
endforeach()

file(GLOB_RECURSE UI_FILES
    "*.ui"
)

qt4_wrap_cpp(WRAPED_SRC_FILES ${SRC_FILES_TO_WARP})

qt4_wrap_ui(WRAPED_UI_FILES ${UI_FILES})


add_executable(${PROJECT_NAME} ${SRC_FILES} ${WRAPED_UI_FILES} ${WRAPED_SRC_FILES})

## Add cmake target dependencies of the executable
## same as for the library above
add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}
    ${catkin_LIBRARIES}
    ${QT_LIBRARIES}
    ${LIBVLC_LIBRARIES}
    ${EXIV2_LIBRARIES}
)


# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_AUTOMOC OFF)
set(QT_USE_QTGUI TRUE)

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
# install(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables and/or libraries for installation
# install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_node
#   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark cpp header files for installation
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_tradr_uav_fpv.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
