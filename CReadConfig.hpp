#pragma once
#include <string>

// 引数型用 仮宣言
class CImageProcessor;
class CThreadDetector;

// [act]コンフィグファイルで設定するコンフィグ一覧
enum EReadConfigList{
	eBinalyzeThreshold,	// P-タイル法による二値化時の閾値(0-100[%])
	eBlackRate,			// 判定1: 物体判定閾値(値以上か)
	eSquareRatio,		// 判定2: 形状判定閾値(値以上か)
	eBendRate,			// 判定3: 蛇行判定閾値(値未満か)
	eConfigListMax		// 要素の最大値(配列変数生成に使用)
};

// [act]1. コンフィグファイルからコンフィグを読み取る
//		2. 各画像処理クラスに必要なコンフィグを読み出す
class CReadConfig{
	double m_readingList[eConfigListMax];	// 読みだしたコンフィグを格納する配列変数
	const std::string m_configFileName;		// 読みだすコンフィグのファイル名: コンストラクタで指定
public:
	// [act]コンストラクタ 外部から読みだすファイル名であるm_configFileNameの指定を受ける。
	// [prm]pFileName	: コンフィグファイル名
	CReadConfig(std::string pFileName) : m_configFileName(pFileName){};
	// [act]コンフィグファイルから値を読み出す。すべてのコンフィグを読み出せなかった場合エラー終了する。
	bool readFile();
	// [act]指示されたコンフィグの値を取り出す
	double outputConfig(const EReadConfigList pList) const;
};
