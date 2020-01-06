#include "FgControllerToBlendShape.h"

#include "XmlParser/Public/XmlParser.h"
#include "XmlParser/Public/XmlFile.h"
#include "XmlParser/Public/XmlNode.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <Runtime/Engine/Public/EngineUtils.h>
#include <Runtime/Engine/Classes/Engine/StaticMeshActor.h>
#include "Animation/SkeletalMeshActor.h"
#include "Animation/AnimCurveTypes.h"
#include "Components/SkeletalMeshComponent.h"

#include "Kismet/GameplayStatics.h"

FgControllerToBlendShape::FgControllerToBlendShape()
{
	mbInitFlag = false;
}

FgControllerToBlendShape::~FgControllerToBlendShape()
{
	reset();
}

bool FgControllerToBlendShape::Init(const string & filepath, const bool bBamespace)
{
	if (!fs::exists(fs::path(filepath))) return mbInitFlag = false;

	mFileList_Sample.clear();
	mMapString2MM.clear();
	mAllDataMap.clear();

	//std::regex reg("fgs", regex::icase);
	string modelXmlPath = "";
	for (auto& FilePath : fs::directory_iterator(filepath))
	{
		std::string fileExtension = FilePath.path().extension().string();
		if (/*std::regex_match(filename, reg)*/fileExtension == ".fgs")
		{
			mFileList_Sample.push_back(FilePath.path().filename().replace_extension().string());
		}

		//For find model's name
		if (fileExtension == ".xml") modelXmlPath = FilePath.path().string();
	}

	if (mFileList_Sample.size() == 0) return mbInitFlag = false;

	//According the filelist to create the datastruct
	for (auto& var : mFileList_Sample)
	{
		std::shared_ptr<BlendName_Point_Map> bnmap = std::make_shared<BlendName_Point_Map>();
		if (bnmap == nullptr) return mbInitFlag = false;

		mAllDataMap[var.c_str()] = bnmap;

		std::shared_ptr<MinMaxStruct> minmaxStruct = std::make_shared<MinMaxStruct>();
		if (minmaxStruct == nullptr) return mbInitFlag = false;
		
		mMapString2MM[var.c_str()] = minmaxStruct;
	}

	//Insert value to datastruct
	for (auto& variable : mAllDataMap)
	{
		string file = filepath + "//" + string(variable.first.GetPlainANSIString()) + ".fgs";

		auto iter = mMapString2MM.find(variable.first);
		if (iter == mMapString2MM.end()) return mbInitFlag = false;

		ReadTxt_BNMap2(file, *variable.second, *(iter->second));
	}

	if (!fs::exists(modelXmlPath)) return false;

	initMapData(modelXmlPath, bBamespace);

	return mbInitFlag = true;
}

bool FgControllerToBlendShape::initMapData(const string& modelXmlPath, const bool bBamespace)
{
	//Clear Map's data
	mString2ModelIndex.clear();

	int size = mBlendValueMap.size();
	vector<FName> tempVecName;
	tempVecName.resize(size);

	bool bFind = false;
	//GWorld->GetWorld().
	TActorIterator<ASkeletalMeshActor> actorItr = TActorIterator<ASkeletalMeshActor>(GWorld->GetWorld(), ASkeletalMeshActor::StaticClass());
	//USkeletalMesh
	for (actorItr; actorItr; ++actorItr)
	{
		if (actorItr)
		{
			string name = std::string(TCHAR_TO_UTF8(*(actorItr->GetActorLabel())));
			if (name == m_label.name)
			{
				ASkeletalMeshActor* skeletalMesh = *actorItr;
				this->mesh = skeletalMesh->GetSkeletalMeshComponent();

				//Set the Animation Drive Mode
				this->mesh->SetAnimationMode(EAnimationMode::AnimationCustomMode);

				this->mesh->SetUpdateAnimationInEditor(bFind = true);
				if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString((name + string(" is found")).c_str()));
			}
		}
	}
	if(!bFind)
		if(GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString((m_label.name + string(" is found")).c_str()));

	return true;
}

void FgControllerToBlendShape::reset()
{
	mMapString2MM.clear();
	mAllDataMap.clear();
	mBlendValueMap.clear();
	mString2ModelIndex.clear();
}

bool FgControllerToBlendShape::ReadTxt_BNMap2(const string& filename, BlendName_Point_Map & bpmap, MinMaxStruct & MinMaxValue)
{
	vector<std::shared_ptr<Time_Value_Map>> tmpvec;
	vector<int> blendNams;
	int sampling = 0;

	std::ifstream fin(filename, std::ios::in);
	char line[1024] = {};
	int outerIndex = 1;
	//Get data from each row
	try
	{
		while (fin.getline(line, sizeof(line)))
		{
			string strData, temp;
			strData = line;

			//Parse the current line
			stringstream word(strData);
			int inIndex = 1;
			while (std::getline(word, temp, (outerIndex != 1 ? ';' : '\n')))
			{
				vector<string> tempString;
				stringstream wordInner(temp);
				string TempInner;
				while (std::getline(wordInner, TempInner, ',')) tempString.push_back(TempInner);

				//First line to get maxmin value
				if (outerIndex == 1)
				{
					sampling = std::atoi(tempString[0].c_str());
					MinMaxValue.Min = std::atof(tempString[1].c_str());
					MinMaxValue.Max = std::atof(tempString[2].c_str());

				}//Second line to get Controller's name and number
				else if (outerIndex == 2)
				{
					//There may be multiple controllers
					/*May has some defect!!!!!!!!!!!!!*/
					blendNams.push_back(std::atoi(tempString[0].c_str()));
					std::shared_ptr<Time_Value_Map> tp = std::make_shared<Time_Value_Map>();
					if (tp == nullptr) return false;
					tp->resize(sampling);
					std::fill(tp->begin(), tp->end(), 0.0f);
					tmpvec.push_back(tp);
					bpmap.emplace(FName(tempString[1].c_str()), tp);
					mBlendValueMap.emplace(FName(tempString[1].c_str()), std::make_shared<Time_Value_Map>());
				}   //Another line to get each line sample value
				else
				{
					int m = std::atoi(tempString[0].c_str());
					int n = std::atoi(tempString[1].c_str());
					float value = std::atof(tempString[2].c_str());
					auto iter = std::find(blendNams.begin(), blendNams.end(), n);
					if (iter == blendNams.end()) return false;

					if (m < sampling) (*tmpvec[std::distance(blendNams.begin(), iter)])[m] = value;
				}
			}
			outerIndex++;
		}
	}
	catch (std::exception& except)
	{
		cout << except.what() << endl;
		return false;
	}

	return true;
}

int FgControllerToBlendShape::SetControllerValue(FName & ControllerName, float Value, float ValueMin, float ValueMax)
{
	auto MinMaxStc = mMapString2MM.find(ControllerName);
	if (MinMaxStc == mMapString2MM.end())
		return 0;

	ValueMin = (MinMaxStc->second)->Min;
	ValueMax = (MinMaxStc->second)->Max;

	if (Value > ValueMax) Value = ValueMax;

	if (Value < ValueMin) Value = ValueMin;

	auto mapP = mAllDataMap.find(ControllerName);
	if (mapP == mAllDataMap.end())				return -999;

	auto& mapRef = *(mapP->second);
	for (auto& var : mapRef)
	{
		auto& BlendName = var.first;
		auto& ValueMap = *var.second;

		if (ValueMap.size() <= 1)				continue;

		if (ValueMin == ValueMax)
		{
			mBlendValueMap[BlendName]->push_back(ValueMin);
			continue;
		}

		int index = (int)((Value - ValueMin) / (ValueMax - ValueMin) * (ValueMap.size() - 1));

		if (index >= ValueMap.size() || index < 0) continue;
		
		mBlendValueMap[BlendName]->push_back(ValueMap[index]);
	}

	return 0;
}

const map<FName, float>* FgControllerToBlendShape::GetReturnMap()
{
	mRMap.clear();

	for (auto& var : mBlendValueMap)
	{
		float retvalue = 0.0f;

		int NotZeroCount = 0;
		for (auto& varA : *(var.second))
		{
			retvalue += varA;
			if (!(fabs(varA) < FLT_EPSILON)) NotZeroCount++;
		}

		if (var.second->size() > 1) retvalue = retvalue / NotZeroCount;

		mRMap[var.first] = retvalue;
		var.second->clear();
	}

	return &mRMap;
}

void FgControllerToBlendShape::CalculateFacialBlendShape()
{
	if (mTempDatas.size() != mIOSFacialBlendShapeVec.size()) return;

	mTempFacial.resize(mVectorString.size());
	std::fill(mTempFacial.begin(), mTempFacial.end(), 0.0f);

	for (int j = 0; j < mVectorString.size(); j++)
	{
		float sum = 0.0f;
		for (int i = 0; i < mIOSFacialBlendShapeVec.size(); i++)
		{
			sum += (mIOSFacialBlendShapeVec[i][j] * mTempDatas[i]);
		}
		mTempFacial[j] = sum;
	}
}

void FgControllerToBlendShape::EvaluateComponentSpaceInternal(const vector<float>& weight)
{
	mTempDatas.resize(weight.size());
	int i = 0;

	for (auto& value : mTempDatas) value = weight[i++];

	CalculateFacialBlendShape();

	for (int i = 0; i < mVectorString.size(); i++)
	{
		if (i < mTempFacial.size() && (mTempFacial[i] > 0.00001f || mTempFacial[i] < -0.00001f))
		{
			if (0 != SetControllerValue(mVectorString[i], mTempFacial[i])) continue;
		}
	}

	const map<FName, float>* retmap = GetReturnMap();

	if (retmap->size() == 0) return;

	for (auto& iter : *retmap)
	{
		if ((!std::isnan(iter.second))) mesh->SetMorphTarget(iter.first, iter.second);
	}
}

void FgControllerToBlendShape::setCharacter(const characterLabel& label)
{
	m_label.channel = label.channel;
	m_label.name = label.name;
	m_label.retargeterPath = label.retargeterPath;
	m_label.samplingPath = label.samplingPath;
	m_label.use = label.use;
}

const characterLabel& FgControllerToBlendShape::getCharacter()
{
	return m_label;
}