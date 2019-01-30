#include <iostream>
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "CThreadDetector.hpp"
#include "CReadConfig.hpp"

using namespace cv;
// cmdによるシステムの起動方法(複数のファイルを一度に解析することもできる):
// 実行ファイル名 コンフィグファイル名 起動モードパラメータ(-d/-p) サイズパラメータ1(-s,直後のファイルにのみ適用) 読み込みファイル名1 サイズパラメータ2(-s) 読み込みファイル名2 ...
// [ret]0以上: 緯糸切れと判定された画像数(1枚の場合0なら正常、1なら緯糸切れ)
//		   -1: パラメータ設定ミス等の異常終了
int main(int argc, char** argv)
{
	int dataStart = 1;				// 現在のコマンドラインパラメータ読み込み位置
	bool debugMode		 = false;	// debugModeで起動するかのフラグ(-d)
	bool photoMode		 = false;	// photoModeで起動するかのフラグ(-p)
	bool minisizeFlag	 = false;	// bgr形式のファイルを使用する際小さいサイズで読み込むかのフラグ(-s)
	
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
	
	arg1 = std::string(argv[dataStart]);
	if (arg1 == "-d"){
		// -d param: 指定するとシステムをデバッグモードで起動する。処理途中の画像の書き出しと時間計測が行われる
		std::cout << "Launch debug mode." << std::endl;
		debugMode = true; ++dataStart;
	} else if (arg1 == "-p"){
		// -p param: 指定するとシステムをフォトモードで起動する。画像からファイルを読み込み保存・ウィンドウに出力して終了する。
		// ファイルを読み込むだけで画像処理や緯糸切れ判定は行われない。
		std::cout << "Launch photoView mode." << std::endl;
		photoMode = true; ++dataStart;
	}

	// ここでコマンドラインが終わる場合画像ファイル名が指定されていないので指定を促す
	if (argc == dataStart){
		std::cout << "Please set imageName as cmd parameter." << std::endl;
		return -1;
	}

	int result = 0;
	// 指定された各画像に対して緯糸切れ検知を行う。
	// bgrデータの場合、各ファイル名の直前に-sオプションをつけることで320x240サイズの画像を読み込む(デフォルトは640x480)
	for(; dataStart<argc; ++dataStart){
		const std::string data(argv[dataStart]);
		if (data == "-s"){ minisizeFlag = true; continue; }
		std::cout << data << std::endl;
		CThreadDetector detector(data, debugMode, photoMode, minisizeFlag, config);
		result = detector.Detect() ? result : result+1;
		minisizeFlag = false;
	}
	return result;
}
