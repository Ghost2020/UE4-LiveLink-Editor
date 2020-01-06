#include "GlobalObject.h"

//权重队列，初始大小为100
TLockFreePointerListFIFO<std::vector<float>, 100> FaceGood_LiveLink::IOSFacialDataFloatVec;

std::atomic<bool> FaceGood_LiveLink::RunFlag = false;
