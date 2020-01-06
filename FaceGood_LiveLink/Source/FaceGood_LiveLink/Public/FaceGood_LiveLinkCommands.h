/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FaceGood_LiveLinkCommands.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "FaceGood_LiveLinkStyle.h"

class FFaceGood_LiveLinkCommands : public TCommands<FFaceGood_LiveLinkCommands>
{
public:

	FFaceGood_LiveLinkCommands()
		: TCommands<FFaceGood_LiveLinkCommands>(TEXT("FaceGood_LiveLink"), NSLOCTEXT("Contexts", "FaceGood_LiveLink", "FaceGood_LiveLink Plugin"), NAME_None, FFaceGood_LiveLinkStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;

	TSharedPtr<FUICommandInfo> AddSource;
	TSharedPtr<FUICommandInfo> RemoveSource;
	TSharedPtr<FUICommandInfo> RemoveAllSources;
	TSharedPtr<FUICommandInfo> ImportAllSource;
	TSharedPtr<FUICommandInfo> ExportAllSources;
};