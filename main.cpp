#include <iostream>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "CThreadDetector.hpp"
using namespace cv;
int main(int argc, char** argv)
{
	int dataStart = 1;	// current parameter pos
	bool debugMode = false;	// whether launch debug mode
	bool photoMode = false;

	// -d param: launch debug mode(write debug images)
	const std::string arg1(argv[dataStart]);
	if (arg1 == "-d"){
		std::cout << "Launch debug mode." << std::endl;
		debugMode = true; ++dataStart;
	} else if (arg1 == "-p"){
		std::cout << "Launch photoView mode." << std::endl;
		photoMode = true; ++dataStart;
	}

	// check detect images are exist.
	if (argc == dataStart){
		std::cout << "Please set imageName as cmd parameter." << std::endl;
		return -1;
	}

	// start detection for each images
	for(; dataStart<argc; ++dataStart){
		std::cout << argv[dataStart] << std::endl;
		CThreadDetector detector(argv[dataStart], debugMode, photoMode);
		detector.Detect();
	}
	return 0;
}
