/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FaceGood_LiveLink.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateStyleRegistry.h"

class FToolBarBuilder;
class FMenuBuilder;

class FFaceGood_LiveLinkRuntimeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override;
};
