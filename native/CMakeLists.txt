cmake_minimum_required( VERSION 3.1.0 FATAL_ERROR )
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_BUILD_TYPE Release )

PROJECT( faceChanger )

include( ~/external/dlib-19.2/dlib/cmake )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY "lib/" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "lib/" )

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${PROJECT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

file( GLOB SOURCES "*.cpp" )

add_executable( faceChanger ${SOURCES} )
target_link_libraries( faceChanger dlib ${OpenCV_LIBS} )
