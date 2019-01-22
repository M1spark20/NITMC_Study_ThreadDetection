#pragma once
#include <opencv2/opencv.hpp>
#include <string>

// クラスの事前宣言
class CReadConfig;

// [act]撮影画像から緯糸を検出する
//		[ReadImage関数]	- 画像読み込み部
//			1. ファイルからカラー画像を読み取りデコードする。対応フォーマットはメジャーなもの+bgr形式である。
//			2. カラー画像をモノクロ画像に変換する
//		[Detect関数] - 画像処理をCImageProcessorクラスにて行ったうえでCheckThread関数にデータを渡し緯糸切れ判定を行う。具体的には:
//			1. Y-prewittフィルタによる画像微分を行い、画像縦方向のエッジを検出する。
//			2. P-タイル法により微分画像の二値化を行う。意味のある画素は黒画素で表現する。
//			3. 二値画像の黒画素を4近傍の連結成分ごとに分け、面積が最大のもののみを取り出す。
//			4. 緯糸の太さを算出すために画像回転補正を行う。補正後の連結成分を緯糸候補成分と呼ぶ。
//		[CheckThread関数] - 直線状の緯糸が存在するかを以下の3つの基準で判定する。
//			1. ノイズ除去前後の二値画像の黒画素数を比較し、ノイズ除去後に残った黒画素数の割合を算出する: 物体判定J1
//			2. 緯糸候補成分の外接長方形の縦横比を算出する: 形状判定J2
//			3. 緯糸候補成分の黒画素数と外接長方形の面積の比を算出する: 蛇行判定J3
class CThreadDetector{
	cv::Mat mProcessImage;					// 画像データ格納先
	std::string mImageFileName;				// 読み込みファイル名
	std::string mExtension;					// 読み込みファイル拡張子(コンストラクタで抜き出す)
	static const std::string mcDebugFolder;	// デバッグ用画像出力先
	bool mIsDebugMode;						// デバッグモードフラグ(処理途中画像の書き出し・処理時間の書き出しを行う)
	bool mIsPhotoMode;						// 画像閲覧モードフラグ(ファイルを読み込んで表示するだけのモード)
	bool mMinisizeImageFlag;				// 小サイズ画像フラグ(読み込む画像のサイズを指定: true-320x240, false-640x480)
	
	// 画像処理・緯糸切れ判定パラメータ一覧(param.cfgを指定)
	const double c_binalyzeRate;			// P-タイル法による二値化時の黒画素数の割合
	const double c_blackRate;				// 判定量J1における閾値: 高ければ高いほどノイズ量が少ない
	const double c_squareRatio;				// 判定量J2における閾値: 高ければ高いほど外接長方形の縦横比が大きい
	const double c_thickRatio;				// 判定量J3における閾値: 低ければ低いほど緯糸は直線状である

	// [act]変数mImageFileNameを用いてファイル読み込みを行う
	bool ReadImage(cv::Mat& dstImage);
	
	// [act]画像処理を施した二値画像を評価し緯糸切れ判定を行う。
	bool CheckThread(const cv::Mat& pBinaryImage, const float pDefRate);
	
	// [act]列データを入力し、その中に白画素があるか判定を行う
	float CheckSegment(const cv::Mat& pRowData);
public:
	// コンストラクタ
	CThreadDetector(const std::string pImageName, bool pIsDebugMode, bool pIsPhotoMode, bool pMinisizeFlag, const CReadConfig& pConfig);
	
	// [act]画像回転補正および緯糸切れ判定を行う
	bool Detect();
};
