cmake_minimum_required(VERSION 3.24.2)
project(digital_image_fundamentals)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# 临近插值算法
add_executable(nearest nearest.cpp)
target_link_libraries(nearest ${OpenCV_LIBS})

# 双线性插值算法
add_executable(linear linear.cpp)
target_link_libraries(linear ${OpenCV_LIBS})

# 双立方插值算法
add_executable(cubic cubic.cpp)
target_link_libraries(cubic ${OpenCV_LIBS})