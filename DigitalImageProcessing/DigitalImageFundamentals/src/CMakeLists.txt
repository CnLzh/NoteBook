cmake_minimum_required(VERSION 3.25)
project(digital_image_fundamentals)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 临近插值算法
add_executable(nearest nearest.cpp)
target_link_libraries(nearest ${OpenCV_LIBS})