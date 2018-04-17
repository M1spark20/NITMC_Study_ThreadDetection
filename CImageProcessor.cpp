#include "CImageProcessor.hpp"
#include <array>
#include <fstream>

// CImageProcessor::CopyMatRect()
// [act]指定範囲のMatをコピーする
// [prm]pSrc	: データコピー元
//		pDst	: データコピー先
//		pSrcRect: srcのコピー範囲(x,y,w,h)
//		pDstRect: dstのコピー範囲(x,y,w,h)
void CImageProcessor::CopyMatRect(const cv::Mat& pSrc, cv::Mat& pDst, const cv::Rect& pSrcRect, const cv::Rect& pDstRect){
	const cv::Mat srcData(pSrc, pSrcRect);
	cv::Mat dstData(pDst, pDstRect);
	dstData = cv::Scalar(0) + srcData;
	/*std::ofstream ofs2("sift.csv");
	ofs2 << cv::format(pDst, "csv") << std::endl;*/
}

// CImageProcessor::Filter3x3()
// [act]端点処理を含んだ3x3フィルタ(YPrewitt用)
//		端点処理を行うことで図形の端部でノイズが発生しないよう工夫してある
// [prm]pSrcImage	: 処理を行う画像(CV_8UC) (y,x)
//		pDstImage	: 処理結果格納先(CV_32F)
//					  ただし、この画像には[0,255]以外の値を含む可能性がある(負の値も含む)。
//		pFilter3x3	: 処理する3x3のフィルタ(CV_32FC1)
void CImageProcessor::Filter3x3(const cv::Mat& pSrcImage, cv::Mat& pDstImage, const cv::Mat& pFilter3x3){
	// 受け取った画像データをfloat型に変換する
	cv::Mat processData;
	pSrcImage.convertTo(processData, CV_32FC1);
	
	/*	これからやることを図で説明
		temp1は元画像の大きさより上下左右+1
		temp2はtemp1の大きさよりさらに上下左右+1, 画像外側は端点のデータをコピーする
		temp1にtemp2のデータを順に足し上げていくと、端点処理ができる
		すると、添付の画像のように端点処理が行える
		緑四角が画像範囲、青四角が拡張範囲
		赤枠内が有効な取得範囲であり、さらにtemp1を上下左右-1すると出力になる
	*/
	// 元画像のサイズを取得してtemp1とtemp2を生成・初期化
	int row = processData.rows, col = processData.cols;	// row: 行(y), col:列(x)
	cv::Mat temp1(row+2, col+2, CV_32FC1, cv::Scalar(0)), temp2(row+4, col+4, CV_32FC1, cv::Scalar(0));
	//std::cout << processData.type() << ", " << temp1.type() << ", " << temp2.type() << std::endl;
	CopyMatRect(processData, temp2, cv::Rect(    0,     0,   1, row), cv::Rect(    1,     2,   1, row));
	CopyMatRect(processData, temp2, cv::Rect(col-1,     0,   1, row), cv::Rect(col+2,     2,   1, row));
	CopyMatRect(processData, temp2, cv::Rect(    0,     0, col,   1), cv::Rect(    2,     1, col,   1));
	CopyMatRect(processData, temp2, cv::Rect(    0, row-1, col,   1), cv::Rect(    2, row+2, col,   1));

	/*cv::Mat temp3; temp2.convertTo(temp3, CV_8UC1);
	cv::imshow("Display Image", temp3);
	cv::waitKey();*/
	CopyMatRect( processData, temp2, cv::Rect(0,0,col,row), cv::Rect(2,2,col,row) );
	/*// デバッグ出力
	temp2.convertTo(temp3, CV_8UC1);
	cv::imshow("Display Image", temp3);
	cv::waitKey();*/
	
	// 代入処理
	for(int y=0; y<3; ++y)
	for(int x=0; x<3; ++x){
		const cv::Mat addition = cv::Mat(temp2, cv::Rect(y, x, col+2, row+2));
		// std::cout << processData.type() << ", " << temp1.type() << ", " << temp2.type() << ", " << addition.type() << std::endl;
		temp1 +=  addition * pFilter3x3.at<float>(cv::Point(y,x));
	}

	// 出力処理
	cv::Mat out(temp1, cv::Rect(1, 1, col, row));
	out.copyTo(pDstImage);
}

// 固定しきい値による2値化
void CImageProcessor::BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold){
	cv::Mat processData = pSrcImage <= pThrethold;
	processData.copyTo(pDstImage);
}

// [act]Pタイル法による2値化
//		画素の最大値から指定閾値の率の画素が黒画素となるように2値化を行う
// [prm]pSrcImage	: 2値化を行う画像(CV_[XX]C1 -> CV_8UC1に内部変換)
//		pBlackRate	: 黒画素とする比率 range->[0.0, 1.0]
void CImageProcessor::BinalyzePTile(const cv::Mat& pSrcImage, cv::Mat& pDstImage, double pBlackRate){
	// CV_8UC1に変換
	cv::Mat processData;
	pSrcImage.convertTo(processData, CV_8UC1);

	// 画像サイズを取得して必要な黒画素数を計算
	cv::Size s = processData.size();
	const int order = s.width * s.height * pBlackRate;

	// ヒストグラムの計算
	cv::MatND hist;
	const int bin_nums[] = { 256 }, channels[] = {0};
	const float hist_range[] = { 0, 256 }, *ranges[] = { hist_range };
	cv::calcHist(&processData, 1, channels, cv::Mat(), hist, 1, bin_nums, ranges);
	//std::cout << hist << std::endl;

	// ヒストグラムの上から黒画素数を加算
	// 輝度の高い方から足し上げていき、閾値を超えたところで打ち切り
	// openCVの0および255への飽和処理を活用して2値化した画像を出力
	int bCheck = 0;
	for (int th = 255; th >= 0; --th){
		bCheck += static_cast<int>( hist.at<float>(th) );
		if (bCheck - order >= 0){
			std::cout << "Image Size: " << s.width << " * " << s.height << "\n";
			std::cout << "Pixels Purpose: " << order << ", Black Pixels: " << bCheck << ", Threshold: " << th << "\n";
			BinalyzeThreshold(processData, pDstImage, th);
			return;
		}
	}
}

// [act]ラベリングを行って面積が最大となる部分のみを抽出
// [prm]pSrcImage	: 2値化を行う画像(CV_[XX]C1 -> CV_8UC1に内部変換)
//		pDstImage	: 処理結果格納先(CV_8UC1)
void CImageProcessor::LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage){
	// CV_8UC1に変換し、画像サイズを抽出
	const cv::Size s = pSrcImage.size();
	const int scale = s.width * s.height;

	// 変数定義
	cv::Mat labelMat(s.height + 1, s.width + 1, CV_32SC1, cv::Scalar(0));
	cv::Mat processData(s.height + 1, s.width + 1, CV_8UC1, cv::Scalar(255));
	/* 初期値として元画像をコピー */{
		cv::Mat temp = processData(cv::Rect(1, 1, s.width, s.height));
		CopyMatRect(pSrcImage, processData,
			cv::Rect(0, 0, s.width, s.height), cv::Rect(1, 1, s.width, s.height));
	}
	std::vector<int> labelCounter;
	// labelNo, labelCount
	std::pair<int, int> maxLabel;

	// メイン処理
	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(cv::Scalar(255) - processData, nonZeroList);
	//std::cout << nonZeroList.size() << ", " << scale << std::endl;
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		int *const pDataPtr = &labelMat.at<int>(*it);
		const int tempLabel[] = {
			labelMat.at<int>(*it - cv::Point(1,0)), labelMat.at<int>(*it - cv::Point(0,1))
		};
		//std::cout << "(" << tempLabel[0] << ", " << tempLabel[1] << ") " << *it << " " << (int)*pDataPtr << " " ;
		// 参照画素の左、上ともラベルが付いていない(=0)の場合:
		// 新規ラベル番号を付与する
		if (tempLabel[0] == 0 && tempLabel[1] == 0){
			labelCounter.push_back(1);
			*pDataPtr = (int)labelCounter.size();
			if (maxLabel.second == 0)
				maxLabel = std::pair<int, int>(labelCounter.size(), 1);
			continue;
		}
		// 参照画素の左、上のラベルが同じ or いずれかが付いていない場合:
		// ついていた番号を付与
		if (tempLabel[0] == tempLabel[1] || !tempLabel[0] || !tempLabel[1]) {
			const int setPos = tempLabel[0] ? 0 : 1;
			*pDataPtr = tempLabel[setPos];
			const int nowCount = ++labelCounter[tempLabel[setPos] - 1];
			maxLabel = maxLabel.second < nowCount ?
				std::pair<int, int>(tempLabel[setPos], nowCount) : maxLabel;
			continue;
		}

		// 参照画素の左、上に別々のラベルが付いていた場合:
		// ラベル番号を小さい方に合わせる
		const int moveDir = tempLabel[0] < tempLabel[1] ? 0 : 1;
		*pDataPtr = tempLabel[moveDir];

		// ラベル番号を差し替えるラベル位置を付与
		// compare elements (if true, set to 255, CV_8U) -> 0or1
		cv::Mat replacePos;
		cv::compare(labelMat, tempLabel[1 - moveDir], replacePos, CV_CMP_EQ);
		replacePos /= 255;

		// type convert to CV_16UC1 and saturate to 2^16
		replacePos.convertTo(replacePos, CV_32SC1);
		// ラベル番号の差し替え
		// (2018/01/16)Substract larger number from difference of larger and smaller
		// to make them smaller number. replacePos works as a matrix mask.
		labelMat -= (tempLabel[1-moveDir]-tempLabel[moveDir])*replacePos;

		// ラベル数の加算
		labelCounter[tempLabel[moveDir] - 1] += labelCounter[tempLabel[1 - moveDir] - 1];
		labelCounter[tempLabel[1 - moveDir] - 1] = 0;
		const int nowCount = ++labelCounter[tempLabel[moveDir] - 1];
		maxLabel = maxLabel.second < nowCount ?
			std::pair<int, int>(tempLabel[moveDir], nowCount) : maxLabel;
	}
	//for(int i=0; i<labelCounter.size(); ++i)
	//	std::cout << labelCounter[i] << ", ";
	/*std::ofstream ofs("sift.csv");
	ofs << cv::format(out, "csv") << std::endl;*/

	// 端点処理部分除去
	cv::Mat out(labelMat, cv::Rect(1, 1, s.width, s.height));
	// compare elements (if true, set to 255, CV_8U)
	out = out == (maxLabel.first);
	out.convertTo(out, CV_8UC1);
	out.copyTo(pDstImage);
}
// ((false: 縮小 / true: 拡大 | vector配列で指定)
/*cv::Mat CImageProcessor::MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob){
	return cv::Mat();
}*/
