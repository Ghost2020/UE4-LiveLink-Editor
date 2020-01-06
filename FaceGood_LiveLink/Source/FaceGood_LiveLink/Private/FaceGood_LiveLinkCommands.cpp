/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FaceGood_LiveLinkCommands.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/

#include "FaceGood_LiveLinkCommands.h"

#define LOCTEXT_NAMESPACE "FFaceGood_LiveLinkEditorModule"

void FFaceGood_LiveLinkCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "Live_Link",				 "Bring up FaceGood_LiveLink window",EUserInterfaceActionType::Button, FInputGesture());
															 
	UI_COMMAND(AddSource,		 "Add",						 "Add a new live link source",		 EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RemoveSource,	 "Remove Selected Source(s)","Remove selected live link source", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RemoveAllSources, "Remove All Sources",		 "Remove all live link sources",	 EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ImportAllSource,	 "Import Selected Source(s)","Import all live link Character",	 EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(ExportAllSources, "Import All Sources",		 "Export all live link Character",	 EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
