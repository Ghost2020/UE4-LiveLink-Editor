// Fill out your copyright notice in the Description page of Project Settings.

#include "FgLiveLinkSocket.h"
#include "GlobalObject.h"
#include "QdDataType.h"
#include <thread>

#include "Animation/Skeleton.h"

class USkeleton;
class UWorld;

#define BUFFSIZE (1024 * 1024 * 50)

FSocket* FgLiveLinkSocket::ClientSocket = nullptr;
std::atomic<bool> FgLiveLinkSocket::RunFlag = false;
FIPv4Address FgLiveLinkSocket::ipAddress;
uint8* FgLiveLinkSocket::RecvData = nullptr;

bool FgLiveLinkSocket::connect(FString ip, int32 port)
{
	FIPv4Address::Parse(ip, ipAddress);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ipAddress.Value);
	addr->SetPort(port);

	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	if (ClientSocket->Connect(*addr))
	{
		FgLiveLinkSocket::RunFlag = true;
		static int invoke = 1;
		if (invoke++ == 1)
		{
			RecvData = (uint8*)calloc(BUFFSIZE, 1);
			std::thread tr(&FgLiveLinkSocket::work);
			tr.detach();
		}
		
		return true;
	}
	else
	{
		return FgLiveLinkSocket::RunFlag = false;
	}
}

bool FgLiveLinkSocket::disconnect()
{
	FgLiveLinkSocket::ClientSocket->Close();
	FgLiveLinkSocket::RunFlag = false;

	return true;
}

void FgLiveLinkSocket::work()
{
	while (true)
	{
		while (FgLiveLinkSocket::RunFlag)
		{
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));

			TArray<uint8> ReceiveData;
			uint32 size = 0;
			//if (ClientSocket->HasPendingData(size))
			{
				int32 read = 0;
				ClientSocket->Recv(RecvData, BUFFSIZE, read);

				uint32 walker = 0;
				int32 Mode = -1;

				while (walker < static_cast<uint32>(read))
				{
					if (RecvData[walker++] != 'F')			continue;

					if (RecvData[walker++] != 'G')			continue;

					Mode = (RecvData[walker] & 0xFF) * 0x10 + (RecvData[walker + 1] & 0xFF);

					if (Mode < 0 || Mode >= QdDataType::Max)continue;

					walker += 2;

					auto a = RecvData[walker];
					auto b = RecvData[walker + 1];
					auto c = RecvData[walker + 2];
					auto d = RecvData[walker + 3];

					uint32 len = a * 0x1000000 + b * 0x10000 + c * 0x100 + d;
					if (len > (uint32)read - walker)		continue;

					walker += 4;

					if (RecvData[walker + len] != 0xF0)		continue;

					//½â°ü
					switch (Mode)
					{
						case QdDataType::Ios_Facial:
						{
							float *fb = (float*)(RecvData + walker);

							std::vector<float> tempIosFacialVec;
							tempIosFacialVec.resize(len / sizeof(float));

							memcpy(tempIosFacialVec.data(), fb, len);
							walker += len;

							FaceGood_LiveLink::IOSFacialDataFloatVec.Push(&tempIosFacialVec);
						}
					}
					walker++;
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

#undef BUFFSIZE