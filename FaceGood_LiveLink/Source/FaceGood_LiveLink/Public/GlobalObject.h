/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               GlobalObject.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/
#pragma once

#include "LockFreeList.h"
#include <vector>
#include <atomic>

namespace FaceGood_LiveLink
{
	extern std::atomic<bool> RunFlag;
	extern TLockFreePointerListFIFO<std::vector<float>, 100> IOSFacialDataFloatVec;
};