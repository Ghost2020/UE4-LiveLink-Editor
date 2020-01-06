/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FgControllerToBlendShape.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <list>
#include <filesystem>
#include <exception>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <mutex>

#include <Engine/World.h>

using namespace std;
namespace fs = std::filesystem;

struct characterLabel
{
	int channel;
	string name;
	string retargeterPath;
	string samplingPath;
	bool use;
};

class FgControllerToBlendShape
{
	friend class FgLiveLinkPanel;


	typedef vector<float> Time_Value_Map;
	typedef map<FName, std::shared_ptr<Time_Value_Map>> BlendName_Point_Map;

	struct MinMaxStruct
	{
		float Min;
		float Max;
	};

	struct ModelIndex
	{
		int		 index;
		USkeleton::AnimCurveUID uid;
	};

public:
	explicit FgControllerToBlendShape();
	~FgControllerToBlendShape();

	FgControllerToBlendShape& operator=(const FgControllerToBlendShape&) = delete;
	FgControllerToBlendShape(const FgControllerToBlendShape &) = delete;

public:
	bool Init(const string& filepath, const bool bBamespace = false);
	bool initMapData(const string& modelXmlPath, const bool bBamespace);
	void reset();

	void setCharacter(const characterLabel& label);
	const characterLabel& getCharacter();

private:
	bool ReadTxt_BNMap2(const string& filename, BlendName_Point_Map& bpmap, MinMaxStruct& MinMaxValue);
	void CalculateFacialBlendShape();
	const map<FName, float>* GetReturnMap();
	int SetControllerValue(FName& ControllerName, float Value, float ValueMin = -1.0f, float ValueMax = 1.0f);
	void EvaluateComponentSpaceInternal(const vector<float>& weight);

public:
	vector<FName> mVectorString;
	vector<vector<float>> mIOSFacialBlendShapeVec;

private:
	vector<string> mFileList_Sample;

	//Controller Minmum and Maxmum's value
	map<FName, std::shared_ptr<MinMaxStruct>> mMapString2MM;
	map<FName, std::shared_ptr<BlendName_Point_Map>> mAllDataMap;
	map<FName, std::shared_ptr<Time_Value_Map>> mBlendValueMap;
	map<FName, std::shared_ptr<ModelIndex>> mString2ModelIndex;
	map<FName, float> mRMap;

	list<string> mListExpression;

	vector<float> mTempDatas;
	vector<float> mTempFacial;
	vector<FName> mVectorModel;

	//FAnimationEvaluationContext
	FBlendedHeapCurve curve;

	bool		 mbInitFlag;

	characterLabel m_label;

	USkeletalMeshComponent* mesh;
};