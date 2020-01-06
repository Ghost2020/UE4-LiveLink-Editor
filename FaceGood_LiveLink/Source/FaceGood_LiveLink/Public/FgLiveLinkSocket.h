// Fill out your copyright notice in the Description page of Project Settings.

/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FgLiveLinkSocket.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/
#pragma once

#include "CoreMinimal.h"
#include "Networking/Public/Networking.h"
#include <atomic>

class FgLiveLinkSocket final
{
public:
	FgLiveLinkSocket() {}
	~FgLiveLinkSocket(){}

	static bool connect(FString ip, int32 port);
	static bool disconnect();
	static void work();

private:
	static std::atomic<bool> RunFlag;
	static uint8* RecvData;
	static FSocket* ClientSocket;
	static FIPv4Address ipAddress;
};
