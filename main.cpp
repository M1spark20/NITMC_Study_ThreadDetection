#include <iostream>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "CThreadDetector.hpp"
using namespace cv;
int main(int argc, char** argv)
{
	if (argc == 1){
		std::cout << "Please set imageName as cmd parameter." << std::endl;
		return -1;
	}
	for(int i=1; i<argc; ++i){
		std::cout << argv[i] << std::endl;
		CThreadDetector detector(argv[i]);
		detector.Detect();
	}
	return 0;
}
