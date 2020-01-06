/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FaceGood_LiveLink.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/

#include "FaceGood_LiveLinkRuntime.h"
#include "Interfaces/IPluginManager.h"

#include "Modules/ModuleManager.h"
#include "Misc/CoreDelegates.h"

#include <iostream>


#define LOCTEXT_NAMESPACE "FFaceGood_LiveLinkRuntimeModule"

void FFaceGood_LiveLinkRuntimeModule::StartupModule()
{
	
}

void FFaceGood_LiveLinkRuntimeModule::ShutdownModule()
{
	
}

bool FFaceGood_LiveLinkRuntimeModule::SupportsDynamicReloading()
{
	return false;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFaceGood_LiveLinkRuntimeModule, FaceGood_LiveLinkRuntime)