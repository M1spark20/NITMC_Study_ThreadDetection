# generated at 13:41:26 2017/12/19
cmake_minimum_required(VERSION 2.8)

project( threadDetect )

find_package( OpenCV REQUIRED )

# To use OpenCV headers
include_directories( ${OpenCV_INCLUDE_DIRS} )

# cmake will add header files automatically
add_executable( threadDetect 
	main.cpp
	CImageProcessor.cpp
	CThreadDetector.cpp
)

#To use OpenCV static libraries
target_link_libraries( threadDetect ${OpenCV_LIBS} )
