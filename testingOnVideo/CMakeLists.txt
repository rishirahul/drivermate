cmake_minimum_required(VERSION 2.8)
project( videoImprove )
set(CMAKE_BUILD_TYPE Debug)
find_package( OpenCV REQUIRED )
add_executable( videoImprove coreCode.cpp )
target_link_libraries( videoImprove ${OpenCV_LIBS} )
