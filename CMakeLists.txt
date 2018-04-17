# generated at 13:41:26 2017/12/19
# modified at 14:04:32 2018/04/17 which is for using C++11

cmake_minimum_required(VERSION 3.1)

project( threadDetect )

# add_definitions(-std=c++0x)
set (CMAKE_CXX_STANDARD 11)

find_package( OpenCV REQUIRED )

# To use OpenCV headers
include_directories( ${OpenCV_INCLUDE_DIRS} )

# cmake will add header files automatically
add_executable( threadDetect 
	main.cpp
	CImageProcessor.cpp
	CThreadDetector.cpp
	CTimeMeasure.cpp
)

#To use OpenCV static libraries
target_link_libraries( threadDetect ${OpenCV_LIBS} )
