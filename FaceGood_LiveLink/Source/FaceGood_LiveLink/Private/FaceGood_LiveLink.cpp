/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FaceGood_LiveLink.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/

#include "FaceGood_LiveLink.h"
#include "Interfaces/IPluginManager.h"

#include "Editor.h"

#include "Modules/ModuleManager.h"
#include "Misc/CoreDelegates.h"
#include "Widgets/Docking/SDockTab.h"

#include "EditorStyleSet.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "MultiBoxBuilder.h"

#include "Features/IModularFeatures.h"
#include "LevelEditor.h"

#include "FgLiveLinkPanel.h"
#include "FaceGood_LiveLinkCommands.h"

#include "GlobalObject.h"

static const FName FaceGood_LiveLinkTabName(TEXT("Fg_LiveLink"));
static const FName FaceGood_LevelEditorModuleName(TEXT("Fg_LevelEditor"));

#define LOCTEXT_NAMESPACE "FFaceGood_LiveLinkEditorModule"

#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( InPluginContent( RelativePath, ".png" ), __VA_ARGS__ )

FString InPluginContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("FaceGood_LiveLink"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

void FFaceGood_LiveLinkEditorModule::StartupModule()
{
	static FName LiveLinkStyle(TEXT("FgLiveLinkStyle"));
	StyleSet = MakeShareable(new FSlateStyleSet(LiveLinkStyle));

	if (!FModuleManager::Get().IsModuleLoaded(FaceGood_LiveLinkTabName))
	{
		RegisterTab();
	}
	else
	{
		ModulesChangedHandle = FModuleManager::Get().OnModulesChanged().AddRaw(this, &FFaceGood_LiveLinkEditorModule::ModulesChangesCallback);
	}

	FFaceGood_LiveLinkCommands::Register();

	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);

	StyleSet->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleSet->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	StyleSet->Set("LiveLink.Common.Icon",				new IMAGE_PLUGIN_BRUSH(TEXT("Fg_LiveLink_40x"),			 Icon40x40));
	StyleSet->Set("LiveLink.Common.Icon.Small",			new IMAGE_PLUGIN_BRUSH(TEXT("Fg_LiveLink_16x"),			 Icon16x16));																											 
	StyleSet->Set("LiveLink.Common.AddSource",			new IMAGE_PLUGIN_BRUSH(TEXT("Fg_icon_AddSource_40x"),	 Icon40x40));
	StyleSet->Set("LiveLink.Common.RemoveSource",		new IMAGE_PLUGIN_BRUSH(TEXT("Fg_icon_RemoveSource_40x"), Icon40x40));
	StyleSet->Set("LiveLink.Common.RemoveAllSources",	new IMAGE_PLUGIN_BRUSH(TEXT("Fg_icon_RemoveAllSource_40x"), Icon40x40));
	StyleSet->Set("LiveLink.Common.ImportAllSource",	new IMAGE_PLUGIN_BRUSH(TEXT("Fg_LiveLink_Import_Source"), Icon40x40));
	StyleSet->Set("LiveLink.Common.ExportAllSources",	new IMAGE_PLUGIN_BRUSH(TEXT("Fg_LiveLink_Export_Source"), Icon40x40));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());

	FaceGood_LiveLink::RunFlag = true;
}

void FFaceGood_LiveLinkEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FFaceGood_LiveLinkStyle::Shutdown();

	FFaceGood_LiveLinkCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FaceGood_LiveLinkTabName);
}

bool FFaceGood_LiveLinkEditorModule::SupportsDynamicReloading()
{
	return false;
}

void FFaceGood_LiveLinkEditorModule::ModulesChangesCallback(FName ModuleName, EModuleChangeReason ReasonForChange)
{
	if (ReasonForChange == EModuleChangeReason::ModuleLoaded && ModuleName == FaceGood_LiveLinkTabName)
	{
		RegisterTab();
	}
}

TSharedRef<SDockTab> FFaceGood_LiveLinkEditorModule::SpawnLiveLinkTab(const FSpawnTabArgs& SpawnTabArgs, TSharedPtr<FSlateStyleSet> StyleSet)
{
	//FLiveLinkClient* Client = &IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(FLiveLinkClient::ModularFeatureName);

	const FSlateBrush* IconBrush = StyleSet->GetBrush("LiveLink.Common.Icon.Small");

	const TSharedRef<SDockTab> MajorTab = SNew(SDockTab).Icon(IconBrush).TabRole(ETabRole::NomadTab);

	MajorTab->SetContent(SNew(FgLiveLinkPanel));
	
	auto onClosed = [](TSharedRef<SDockTab>)
	{
		FaceGood_LiveLink::RunFlag = false;
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString("The FaceGood Live Link Plugin being destroyed"));
	};
	// Need to add more protect
	MajorTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(onClosed));

	return MajorTab;
}

void FFaceGood_LiveLinkEditorModule::closing()
{
	
}

void FFaceGood_LiveLinkEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(FaceGood_LiveLinkTabName);
}

TSharedPtr<class ISlateStyle> FFaceGood_LiveLinkEditorModule::GetStyleSet()
{
	 return StyleSet; 
}

void FFaceGood_LiveLinkEditorModule::RegisterTab()
{
	FFaceGood_LiveLinkStyle::Initialize();
	FFaceGood_LiveLinkStyle::ReloadTextures();
	FFaceGood_LiveLinkCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	TSharedPtr<FSlateStyleSet> StyleSetPtr = StyleSet;

	PluginCommands->MapAction(
		FFaceGood_LiveLinkCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FFaceGood_LiveLinkEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("LevelEditor", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateRaw(this, &FFaceGood_LiveLinkEditorModule::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FFaceGood_LiveLinkEditorModule::AddToolbarExtension));

		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FaceGood_LiveLinkTabName, FOnSpawnTab::CreateStatic(&FFaceGood_LiveLinkEditorModule::SpawnLiveLinkTab, StyleSetPtr))
					  .SetDisplayName(LOCTEXT("FFaceGood_LiveLinkTabTitle", "LiveLink"))
					  .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FFaceGood_LiveLinkEditorModule::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FFaceGood_LiveLinkCommands::Get().OpenPluginWindow);
}

void FFaceGood_LiveLinkEditorModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FFaceGood_LiveLinkCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFaceGood_LiveLinkEditorModule, FaceGood_LiveLink)
