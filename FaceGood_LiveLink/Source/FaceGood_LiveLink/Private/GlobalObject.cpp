#include "GlobalObject.h"

//Ȩ�ض��У���ʼ��СΪ100
TLockFreePointerListFIFO<std::vector<float>, 100> FaceGood_LiveLink::IOSFacialDataFloatVec;

std::atomic<bool> FaceGood_LiveLink::RunFlag = false;
