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

class FFaceGood_LiveLinkEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool SupportsDynamicReloading() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

	TSharedPtr< class ISlateStyle > GetStyleSet();
	
private:
	void RegisterTab();
	void ModulesChangesCallback(FName ModuleName, EModuleChangeReason ReasonForChange);
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	void closing();

	static TSharedRef<SDockTab> SpawnLiveLinkTab(const FSpawnTabArgs& SpawnTabArgs, TSharedPtr<FSlateStyleSet> StyleSet);

public:
	TSharedPtr<FSlateStyleSet> StyleSet;

private:
	TSharedPtr<class FUICommandList> PluginCommands;

	FDelegateHandle LevelEditorTabManagerChangedHandle;
	FDelegateHandle ModulesChangedHandle;
};
