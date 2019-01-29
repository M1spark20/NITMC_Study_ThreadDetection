#include "CThreadDetector.hpp"
#include "CImageProcessor.hpp"
#include "CTimeMeasure.hpp"
#include "CReadConfig.hpp"
#include <fstream>
#include <iomanip>
#include <numeric>

// 定数であるデバッグファイル出力先をクラス外で定義する:
const std::string CThreadDetector::mcDebugFolder = "debugImage/";

// [act]パラメータとファイル名を用いてCThreadDetectorクラスの初期化を行う。
//		初期化ではフラグの設定および指定ファイル名を名前と拡張子に分離する。
// [prm]pImageName		: 拡張子を含んだ画像保存先パス
//		pIsDebugMode	: debugModeでシステムを起動するかどうか
//		pIsPhotoMode	: photoModeでシステムを起動するかどうか
//		pMinisizeFlag	: bgr形式の画像において小さい画像サイズで処理を行うかどうか(true: 320x240, false: 640x480)
//		pConfig			: 緯糸切れ判定に使用するコンフィグ一覧
// [ret]なし
CThreadDetector::CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode, bool pMinisizeFlag, const CReadConfig& pConfig) : 
		// ↓初期化子によるクラス定数メンバの初期化 - pConfigからパラメータを読み出す
		c_binalyzeRate(pConfig.outputConfig(eBinalyzeThreshold)),
		c_blackRate(pConfig.outputConfig(eBlackRate)),
		c_squareRatio(pConfig.outputConfig(eSquareRatio)),
		c_thickRatio(pConfig.outputConfig(eBendRate)){
	// フラグの設定を行う
	mIsDebugMode		= pIsDebugMode;
	mIsPhotoMode		= pIsPhotoMode;
	mMinisizeImageFlag	= pMinisizeFlag;
	// 拡張子の判定: std::string::find()で拡張子を指すドットを探す
	const std::string::size_type dotPos = pImageName.find(".");
	// 拡張子が見つからない場合: すべての名前を書き出し
	if(dotPos == std::string::npos)
		mImageFileName = pImageName;
	else {
		// 拡張子が見つかった場合: ファイル名を名前と拡張子に分離
		mImageFileName = pImageName.substr(0, dotPos);
		mExtension = pImageName.substr(dotPos);
	}
}

// [act]指定されたファイルから画像を読み込む。
//		サポートは一般的な画像ファイルおよびbgr形式
//		読み込んだ画像をグレースケール化するまでこの関数が担当する
// [prm]dstImage: 出力画像(CV_8UC1)
// [ret]読込に成功したか(true:成功, false:失敗)
bool CThreadDetector::ReadImage(cv::Mat& dstImage){
	if (mExtension == ".bgr"){
		// bgr形式の場合
		// パラメータに応じて画像サイズを指定
		const int width  = 640 / (mMinisizeImageFlag ? 2:1);
		const int height = 480 / (mMinisizeImageFlag ? 2:1);
		const int pixel = width*height, colorNum = 3;
		
		// ファイルをバイナリ形式で読込 / 読込失敗時はfalseを返して終了
		std::ifstream ifs(mImageFileName + mExtension, std::ios::binary);
		if (!ifs) return false;
		
		// 格納先cv::Mat生成
		dstImage = cv::Mat(height, width, CV_8UC3);
		// 読込サイズ計算
		const int readSize = sizeof(*dstImage.data)*pixel*colorNum;
		// デバッグ・画像閲覧モードの場合ファイルサイズを書き出す
		if(mIsDebugMode | mIsPhotoMode)
			std::cout << ".bgr dataSize[byte]: " << readSize << std::endl;
		// ファイルの内容を使用する画像のサイズ分cv::Mat::data構造体に直接書き出す
		// bgr形式とcv::Mat::data構造体のフォーマットは同一である
		// 画像サイズが同一であればファイルのすべてが読み込まれる処理となる
		ifs.read((char*)dstImage.data, readSize);
		// デバッグ用
		/*std::ofstream ofs("sift.csv");
		ofs << cv::format(dstImage, "csv") << std::endl;*/
	}
	else {
		// bgr形式以外の場合
		dstImage = cv::imread(mImageFileName + mExtension);
	}
	// グレースケール画像に変換 計算式は輝度Y=R*0.299+G*0.587+B*0.114
	// 内部的にはシフトレジスタを使用して整数のまま処理を行っているらしい
	cv::cvtColor(dstImage, dstImage, CV_RGB2GRAY);
	// cv::MatタイプをCV_8UC1に変換して出力先に書き出し
	dstImage.convertTo(dstImage, CV_8UC1);
	return true;
}

// [act]画像処理・緯糸切れ判定のメインフローを扱う
//		画像読み込み→画像処理→緯糸切れ判定
// [ret]緯糸が切れていなかったかどうか(true:切れていない, false:切れている)
bool CThreadDetector::Detect(){
	CImageProcessor processor;					// 画像処理担当クラス
	cv::Mat procImage;							// 処理画像格納先
	CTimeMeasure timeManager;					// 時間管理担当クラス(debugMode/photoModeで使用)
	const float binalyzeRate = c_binalyzeRate;	// 画像二値化時の閾値定義

	// debugMode/photoModeであれば時間計測開始
	if(mIsDebugMode | mIsPhotoMode) timeManager.StartInput();

	// 画像読み込み・グレースケール化(読み込みに失敗した場合は緯糸切れ判定としてfalseを返す)
	ReadImage(procImage);
	if (procImage.empty()) return false;
	// 所要時間を記録する・読み込み画像を書き出す(bmp形式)
	if (mIsDebugMode | mIsPhotoMode){
		timeManager.GoToNextSection("Read Image Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#0.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// photoModeならここで所要時間を書き出して待機、キー入力で終了
	if (mIsPhotoMode) {
		timeManager.WriteResult();
		cv::imshow("photoViewer : press any key to exit.", procImage);
		cv::waitKey();
		return true;
	}

	// 3x3のY-prewittフィルタをかけ画像縦方向の微分を行う
	processor.Filter3x3(procImage, procImage, (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1));
	// 所要時間を記録する
	if (mIsDebugMode) timeManager.GoToNextSection("Y-Prewitt Filter Time");

	// P-タイル法により二値化を行う
	processor.BinalyzePTile(procImage, procImage, binalyzeRate);
	// 所要時間を記録する・二値画像を書き出す(bmp形式)
	if (mIsDebugMode){
		timeManager.GoToNextSection("P-Tile Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#1.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// ラベリングにより最大連結成分のみを取り出してノイズ除去を行う
	// 内部的にここの画像は連結成分が白色で表現される:findNonZeroを使用するため
	processor.LaberingMaxSize(procImage, procImage);
	// 所要時間を記録する・ノイズ除去後の画像を書き出す(bmp形式)
	if (mIsDebugMode){
		timeManager.GoToNextSection("Labeling Time");
		cv::imwrite(mcDebugFolder + mImageFileName + "#2.bmp", procImage);
		timeManager.GoToNextSection("");
	}

	// 取り出した連結成分を白画素とする
	// 画像回転補正及び緯糸切れ判定を行う
	bool ans = CheckThread(procImage, binalyzeRate);
	// 所要時間を記録してコンソールに書き出す
	if (mIsDebugMode){
		timeManager.GoToNextSection("Detecting Time");
		timeManager.WriteResult();
	}
	
	// コンソールにおいてデータを区切るために改行を入れる
	std::cout << "\n";
	
	// 判定結果を返す
	return ans;
}

// [act]画像回転補正および緯糸切れ判定を行う。
//		手順として、まず判定量J1を計算し物体判定を行う。
//		次にアフィン変換による画像回転補正を行い、外接長方形の四隅の点の座標を取得する。
//		この理由は緯糸切れ判定において外接長方形関連で必要となる辺長X,Yは上記データで決定できるためである。
//		debugModeでのみ実際の回転補正結果を画像に出力する。
//		その後、得られた外接長方形の辺長X,Yを利用して判定量J2およびJ3を計算し形状判定及び蛇行判定を行う
// [prm]pBinaryImage	: 判定対象となる二値画像->ラベリングによりノイズ除去を行った二値画像・意味のある画素は白画素
//		pDefRate		: P-タイル法の二値化閾値[0.0, 1.0]->P-タイル法による二値画像の黒画素数NAを計算するために使用
// [ret]緯糸が切れていなかったかどうか(true:切れていない, false:切れている)
bool CThreadDetector::CheckThread(const cv::Mat& pBinaryImage, const float pDefRate){
	// 判定画像の大きさ[pixel]を取得
	const cv::Size imageSizeData = pBinaryImage.size();
	// 判定画像の面積[pixel^2]を計算
	const float size = static_cast<float>(imageSizeData.width * imageSizeData.height);
	// 画像回転補正後二値画像書き出し用変数(debugMode時のみ全画素を白として初期化・使用)
	cv::Mat debugImage;
	if(mIsDebugMode)
		debugImage = cv::Mat(imageSizeData, CV_8UC1, cv::Scalar(255));

	// 二値画像の白画素(有意成分)の座標一覧を取得する
	// nonZeroListの要素数はそのまま有意画素数となり、黒画素数Nとなる
	std::vector<cv::Point> nonZeroList;
	cv::findNonZero(pBinaryImage, nonZeroList);
	
	// 判定量J1の計算: J1 = N/NA  (緯糸候補成分の黒画素数 / P-タイル法による二値画像の黒画素数)
	const float blackNum = static_cast<float>(nonZeroList.size());	// N
	const float blackRate = (blackNum / size) / pDefRate;			// J1 = N/NA
	// 計算条件・判定条件・判定量J1の書き出し
	std::cout << "BinalyzeRate: " << std::setprecision(3) << pDefRate*100.f << "%" << std::endl;
	std::cout << "decreaseRate: " << std::setprecision(3) << blackRate*100.f
		<< "% require: " << std::setprecision(3) << c_blackRate*100.f << "%" << std::endl;
	// 判定量J1が条件を満たさない場合ここで緯糸切れ判定を出力し判定打ち切りできる:
	/*if (blackRate < c_squareRatio){
		std::cout << "      result: NG" << std::endl;
		return false;
	}*/

	// ここから画像回転補正を行う。まず回転角を決定するために連結成分の両端におけるy画素の平均値を計算する
	// 計算は左右両端から中央に向かって画像を走査し、黒画素が見つかった地点で黒画素y座標の平均値を計算。
	// その値を回転補正時の回転角の基準とする。
	
	// 基準点格納先変数を定義
	cv::Point2f sidePoints[2];
	// 画像左側から中央に向かって走査
	for (int x = 0; x < imageSizeData.width; ++x){
		// 画像のうち1列のデータを取得して、y座標の平均値を計算
		const float yPos = CheckSegment(pBinaryImage.col(x));
		if (yPos < 0) continue;	// 見つからなかった(yPos=-1.f)場合1つ右の列を参照する
		// 見つかったら変数に位置を記録する
		sidePoints[0].x = static_cast<float>(x);
		sidePoints[0].y = yPos;
		break;
	}
	// 同様に画像右側から中央に向かって走査
	for (int x = imageSizeData.width-1; x >= 0; --x){
		const float yPos = CheckSegment(pBinaryImage.col(x));
		if (yPos < 0) continue;	// 見つからなかった場合1つ左の列を参照する
		sidePoints[1].x = static_cast<float>(x);
		sidePoints[1].y = yPos;
		break;
	}

	// 回転角を計算するためにarctanを使用するが、その計算に用いる基準点2点のX方向距離, Y方向距離をそれぞれ計算する。
	const cv::Point2f wholeSize(sidePoints[1].x - sidePoints[0].x, sidePoints[1].y - sidePoints[0].y);
	// arctanにより回転角を決定する
	const double wholeAngle = atan2(wholeSize.y, wholeSize.x);
	// 回転角に応じたアフィン変換行列を生成する
	const cv::Mat affineMat = (cv::Mat_<float>(2,2) <<
		cos(wholeAngle), sin(wholeAngle), -sin(wholeAngle), cos(wholeAngle));
	std::cout << "        line: " << sidePoints[0] << " - " << sidePoints[1]
		<< " / angle(deg): " << wholeAngle * 180.f / CV_PI << std::endl;

	// 外接長方形の端点の座標を格納する変数blackRectを定義する: (0:左上, 1:右下)
	cv::Point2f blackRect[2];
	// 各有意画素の座標に対し回転行列を適用する
	for (std::vector<cv::Point>::iterator it = nonZeroList.begin(); it != nonZeroList.end(); ++it){
		const cv::Mat roundedPos = 	affineMat * (cv::Mat_<float>(2,1) << it->x, it->y);
		// 回転後の座標を計算し、外接長方形の最大・最小を更新するならばblackRectを更新する
		// ただしループ初回は無条件に値の代入を行う
		const cv::Point2f check(roundedPos.at<float>(0), roundedPos.at<float>(1));
		if (it == nonZeroList.begin()){
			blackRect[0] = check; blackRect[1] = check;
		} else {
			blackRect[0].x = check.x < blackRect[0].x ? check.x : blackRect[0].x;
			blackRect[0].y = check.y < blackRect[0].y ? check.y : blackRect[0].y;
			blackRect[1].x = check.x > blackRect[1].x ? check.x : blackRect[1].x;
			blackRect[1].y = check.y > blackRect[1].y ? check.y : blackRect[1].y;
		}
		// debugMode時は回転後の座標に黒画素を打つ
		if(!mIsDebugMode) continue;
		const cv::Point2i dp(check);
		if(dp.x<0 || dp.x>=imageSizeData.width) continue;
		if(dp.y+50<0 || dp.y+50>=imageSizeData.height) continue;
		debugImage.at<uchar>(dp.y+50, dp.x) = 0;
	}
	
	// debugMode時に画像回転補正後の二値画像を出力する
	if (mIsDebugMode)
		 cv::imwrite(mcDebugFolder + mImageFileName + "#3.bmp", debugImage);
	
	// 外接長方形の辺長X,Yを計算する
	const cv::Point2f len = blackRect[1] - blackRect[0];
	const float ratio = len.x > len.y ? len.x / len.y : len.y / len.x;					// 判定量J2: max(X,Y)/min(X,Y)
	//const float distance = static_cast<float>(cv::norm(sidePoints[1] - sidePoints[0]));
	//const float expectThick = static_cast<float>(nonZeroList.size()) / distance;
	// 20180131 refact distance: norm->longer length of square (almost equal, norm > longer)
	// equally thick =  shorter / (num of black dot / longer)
	const float expectThick = static_cast<float>(nonZeroList.size()) / (len.x > len.y ? len.x : len.y);
	const float realThick = len.x > len.y ? len.y : len.x;								
	const float thickRatio = len.x * len.y / static_cast<float>(nonZeroList.size());	// 判定量J3: X*Y/N
	// 判定結果書き出し・判定結果を出力して終了
	std::cout << "   convRange: " << std::setprecision(7) << len << std::endl;
	std::cout << "       ratio: " << std::setprecision(3) << ratio
		<< ", require: >=" << std::setprecision(3) << c_squareRatio << std::endl;
	std::cout << "    blackNum: " << std::setprecision(5) << nonZeroList.size() << " /";
	std::cout << " thick: " << "[" << expectThick << ", " << realThick << "]" << std::endl;
	std::cout << "  thickRatio: " << std::setprecision(3) << thickRatio
		<< ", require: <=" << c_thickRatio << std::endl;
	if (ratio < c_squareRatio || blackRate < c_blackRate || thickRatio > c_thickRatio){
		// 緯糸切れ判定
		std::cout << "      result: NG" << std::endl;
		return false;
	} else {
		// 正常判定
		std::cout << "      result: OK!" << std::endl;
		return true;
	}
}

// [act]列データを入力し、その中に白画素があるか判定を行う
//		白画素がある場合は、それらの画素のy座標の平均値を計算し出力する
// [prm]pRowData: 画像内における特定の1列のデータ(有意画素は白画素)
// [ret]入力列に白画素がなかった場合: -1.f, それ以外: 白画素のy座標の平均値
float CThreadDetector::CheckSegment(const cv::Mat& pRowData){
	// 列中の白画素数をカウント・0なら白画素がないと出力して終了
	if(cv::countNonZero(pRowData) == 0) return -1.f;
	// 白画素を持つ要素一覧を取得する
	std::vector<cv::Point> segmentNonZero;
	cv::findNonZero(pRowData, segmentNonZero);
	// 白画素のy座標の平均値を計算する
	// std::accumulate(begin, end, init): 初期値initに設定要素データの足し上げを行う。
	// 白画素数で割ることで平均を取得することができる
	const float test = std::accumulate(segmentNonZero.begin(), segmentNonZero.end(),
		cv::Point(0, 0)).y / static_cast<float>(segmentNonZero.size());
	// y座標の平均値を返す
	return test;
}
