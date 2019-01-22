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

// [act]固定しきい値による2値化
//		閾値の値の画素は白画素となり意味のない画素となる。
// [prm]pSrcImage	: 2値化を行う画像(CV_8UC1 -> CV_8UC1に内部変換)
//		pDstImage	: 二値化を行った画像(CV_8UC1)
//		pThreshold	: 黒画素とする比率 range->[0.0, 1.0] この値を超えない最大の比率が黒画素比率となる
void CImageProcessor::BinalyzeThreshold(const cv::Mat& pSrcImage, cv::Mat& pDstImage, int pThrethold){
	cv::Mat processData = pSrcImage <= pThrethold;
	processData.copyTo(pDstImage);
}

// [act]Pタイル法による2値化
//		画素の最大値から指定閾値の率の画素が黒画素となるように2値化を行う
// [prm]pSrcImage	: 2値化を行う画像(CV_[XX]C1 -> CV_8UC1に内部変換)
//		pDstImage	: 二値化を行った画像(CV_8UC1)
//		pBlackRate	: 黒画素とする比率 range->[0.0, 1.0]
// [ret]実際に書き出された二値画像の黒画素数(pBlackRateで指定された割合の黒画素数未満の値となる)
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
// [prm]pSrcImage	: 2値化を行う画像(CV_8UC1)
//		pDstImage	: 処理結果格納先(CV_8UC1)
void CImageProcessor::LaberingMaxSize(const cv::Mat& pSrcImage, cv::Mat& pDstImage){
	// CV_8UC1に変換し、画像サイズを抽出
	const cv::Size s = pSrcImage.size();
	const int scale = s.width * s.height;

	// 変数定義
	cv::Mat processData(s.height + 1, s.width + 1, CV_8UC1, cv::Scalar(255));
	/* 初期値として元画像をコピー : オーバーフロー防止 */{
		cv::Mat temp = processData(cv::Rect(1, 1, s.width, s.height));
		CopyMatRect(pSrcImage, processData,
			cv::Rect(0, 0, s.width, s.height), cv::Rect(1, 1, s.width, s.height));
	}

	// LabelData = std::vector<std::pair<cv::Point, int>>: 黒画素位置, ラベルIDを格納する変数
	LabelData labelData;
	// 各ラベルの黒画素数をカウントする変数を生成
	std::vector<int> labelCounter;
	// 最大ラベル記録用変数を生成(labelNo, labelCount)
	std::pair<int, int> maxLabel;

	// メイン処理
	// 黒画素の位置を取得し、labelCounter変数にメモリを確保しておく
	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(cv::Scalar(255) - processData, nonZeroList);
	labelCounter.reserve(nonZeroList.size());

	//std::cout << nonZeroList.size() << ", " << scale << std::endl;
	// すべての黒画素に対して処理を行う
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		int putData = 0;			// 今回割り当てるラベル番号
		int tempLabel[] = { 0, 0 };	// 対象画素の左・上の画素のラベル番号
		/* tempLabelの設定: 対象画素の左・上の画素が黒画素か判定し、黒画素なら割り当てられたラベルを取得 */{
			auto nextIt = FindLabelRelative(labelData, *it - cv::Point(1, 0));
			if (nextIt != labelData.rend()) tempLabel[0] = nextIt->second;
			nextIt = FindLabelRelative(labelData, *it - cv::Point(0, 1));
			if (nextIt != labelData.rend()) tempLabel[1] = nextIt->second;
		}
		
		//std::cout << "(" << tempLabel[0] << ", " << tempLabel[1] << ") " << *it << " " << (int)*pDataPtr << " " ;
		// 参照画素の左、上ともラベルが付いていない(=0)の場合:
		// 新規ラベル番号を付与する
		if (tempLabel[0] == 0 && tempLabel[1] == 0){
			labelCounter.push_back(0);
			putData = labelCounter.size();
		}
		// 参照画素の左、上のラベルが同じ or いずれかが付いていない場合:
		// ついていた番号を付与
		else if (tempLabel[0] == tempLabel[1] || !tempLabel[0] || !tempLabel[1]) {
			const int setPos = tempLabel[0] ? 0 : 1;
			putData = tempLabel[setPos];
		}

		// 参照画素の左、上に別々のラベルが付いていた場合:
		// ラベル番号を小さい方に合わせる
		else {
			const int moveDir = tempLabel[0] < tempLabel[1] ? 0 : 1;
			putData = tempLabel[moveDir];
			RefreshLabel(labelData, tempLabel[1 - moveDir], tempLabel[moveDir], labelCounter);
		}

		// ラベル数の加算
		const int nowCount = ++labelCounter[putData - 1];
		maxLabel = maxLabel.second < nowCount ?
			std::pair<int, int>(putData, nowCount) : maxLabel;
		labelData.push_back(std::pair<cv::Point, int>(*it, putData));
	}

	// 端点処理部分を除去した画像を保存
	// 真っ白な画像を生成
	cv::Mat out(s.height, s.width, CV_8UC1, cv::Scalar(255));
	// 最大連結成分のラベル番号を持つ画素を黒画素とする
	for (auto data : labelData){
		if (data.second != maxLabel.first) continue;
		out.at<unsigned char>(data.first - cv::Point(1, 1)) = 0;	// Black: thread data
	}
	// debug出力用
	/*std::ofstream ofs("sift.csv");
	ofs << cv::format(label, "csv") << std::endl;*/
	// 出力先に白黒を反転させて書き出す: 連結成分が白色で表現される
	out = cv::Scalar(255) - out;
	out.convertTo(pDstImage, CV_8UC1);
}

// 未使用
/*cv::Mat CImageProcessor::MinimizeExtend(const cv::Mat pBinaryImage, std::queue<bool> pJob){
	return cv::Mat();
}*/

// [act]LabelDataが整列されていると仮定して、FindForを対象としたマスがdataに存在するか検索する
//		reverse_iteratorを使用することで比較回数の削減を目指す。
// [prm]data	: 比較対象データ = 黒画素座標一覧
//		findFor	: 探す位置
// [ret]データの存在位置 : 存在しない場合はrend()
std::vector<std::pair<cv::Point, int>>::reverse_iterator CImageProcessor::FindLabelRelative(LabelData& data, cv::Point findFor){
	// 比較対象データをすべて探索範囲にする
	for (auto it = data.rbegin(); it != data.rend(); ++it){
		// 取り出したデータの座標がfindForと一致する場合: reverse_iteratorを返して終了
		if (it->first == findFor) return it;
		// 取り出したデータの座標がラスタ走査において探す位置より手前にある場合見つからなかったとしてrend()を返して終了
		if (it->first.x <= findFor.x && it->first.y <= findFor.y) break;
	}
	// ここまでくるという事は見つからなかったという事なのでrend()を返す
	return data.rend();
}

// [act]ラベリングにおける判定終了後のラベル番号の再割り当てを行う
//		そのラベルを持つ黒画素数の値も同時に更新する
// [prm]data			: ラベリングにおいて決定されたラベルデータ(画素ごとに格納される)
//		srcLabel		: 再割り当て元のラベルデータ
//		dstLabel		: 再割り当て先のラベルデータ
//		pLabelCounter	: 各ラベルの黒画素数が格納されたデータ(直接更新を行う)
void CImageProcessor::RefreshLabel(LabelData& data, int srcLabel, int dstLabel, std::vector<int>& pLabelCounter){
	int count = 0;	// ラベルの貼り換えを行った画素数カウンタ
	// 各画素に比較を適用
	for (auto& check : data){
		// 探索がそのラベルが再割り当て対象ならラベルを再割り当て。
		// 割り当て数が再割り当て元の画素数に到達したら比較を終了
		if (check.second == srcLabel){
			check.second = dstLabel;
			if (++count == pLabelCounter[srcLabel - 1]) break;
		}
	}
	// 再割り当て前後のラベルが設定された黒画素の数を更新
	pLabelCounter[dstLabel - 1] += pLabelCounter[srcLabel - 1];
	pLabelCounter[srcLabel - 1] = 0;
}