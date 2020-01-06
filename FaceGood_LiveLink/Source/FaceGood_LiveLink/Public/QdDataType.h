/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               QdDataTypes.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/
#pragma once

#define NAME_MAX_LEN (2 << 5)

namespace QdDataType
{
	enum FaceGoodDataMode
	{
		Bone,
		Facial,
		BoneFacial,
		Control,
		Ios_Facial,
		Max,
	};

	struct BoneData 
	{
		char name[NAME_MAX_LEN];
		char Namespace[NAME_MAX_LEN];

		float x;
		float y;
		float z;

		float qx;
		float qy;
		float qz;
		float qw;
	};


	struct FG_SOCKET_DATA
	{
		char name[NAME_MAX_LEN];
		float value;
	};
};
