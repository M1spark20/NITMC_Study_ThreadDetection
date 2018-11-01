#include <iostream>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "CThreadDetector.hpp"
#include "CReadConfig.hpp"

using namespace cv;
int main(int argc, char** argv)
{
	int dataStart = 1;	// current parameter pos
	bool debugMode		 = false;	// whether launch debug mode
	bool photoMode		 = false;
	bool minisizeFlag	 = false;
	
	if(argc == 1){
		std::cout << "Please set config file path as 1st cmd parameter." << std::endl;
		return -1;
	}
	
	// コンフィグ読み込み
	std::string arg1(argv[dataStart]);
	CReadConfig config(arg1);
	if(!config.readFile()){
		std::cout << "Failed to read all configurations." << std::endl;
		return -1;
	}
	++dataStart;
	
	// -d param: launch debug mode(write debug images)
	arg1 = std::string(argv[dataStart]);
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
	// read .bgr image as 320x240 if "-s" is set before each filenames.
	for(; dataStart<argc; ++dataStart){
		const std::string data(argv[dataStart]);
		if (data == "-s"){ minisizeFlag = true; continue; }
		std::cout << data << std::endl;
		CThreadDetector detector(data, debugMode, photoMode, minisizeFlag, config);
		detector.Detect();
		minisizeFlag = false;
	}
	return 0;
}
