/*

+	Description:            UE4 Live Link with FaceGood Streamer plugin
+	FileName:               FgLiveLinkPanel.h
+	Author:                 Ghost Chen
+   Date:                   2018/11/27

+	Copyright(C)            Quantum Dynamics Lab.
+

*/

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Views/SListView.h"
#include "SlateTypes.h"
#include <string>
#include <atomic>

struct FLiveLinkCharacterUIEntry;
class ULiveLinkSourceFactory;
class ITableRow;
class STableViewBase;
class SCheckBox;
class FMenuBuilder;
class FUICommandList;
class FXmlFile;
class FXmlNode;

typedef TSharedPtr<FLiveLinkCharacterUIEntry> FLiveLinkCharacterUIEntryPtr;

using namespace std;

class FACEGOOD_LIVELINKEDITOR_API FgLiveLinkPanel : public SCompoundWidget, public FGCObject
{
	SLATE_BEGIN_ARGS(FgLiveLinkPanel)
	{
	}

	SLATE_END_ARGS()

	~FgLiveLinkPanel();

	void Construct(const FArguments& Args/*, FLiveLinkClient* InClient*/);

	// FGCObject interface
	void AddReferencedObjects(FReferenceCollector& Collector) override;
	// End of FGCObject interface

private:

	void BindCommands();

	void HandleOnAddFromFactory(ULiveLinkSourceFactory* InSourceFactory);

	// Controls whether the editor performance throttling warning should be visible
	EVisibility ShowEditorPerformanceThrottlingWarning() const;

	void NetStateChanged(ECheckBoxState InState);

	void HandleRemoveSource();
	void HandleRemoveAllSources();
	void HandleImportSource();
	void HandleExportSource();

	void AnimationSwitch(bool open = false);
	void AnimationEngine();

	// Handle click for Configire
	FReply OnClickConfigure();
	// Handle disabling of editor performance throttling
	FReply DisableEditorPerformanceThrottling();
	// Handle Click for HomePage
	FReply OnClickHomePage();
	// Handle Click for Samping
	FReply OnClickBrowseSamping();
	// Handle Click for Retargeter
	FReply OnClickBrowseRetargeter();

private:
	const TArray<FLiveLinkCharacterUIEntryPtr>& GetCurrentSources() const;

	TSharedRef<ITableRow> MakeSourceListViewWidget(FLiveLinkCharacterUIEntryPtr Entry, const TSharedRef<STableViewBase>& OwnerTable) const;

	void OnSourceListSelectionChanged(FLiveLinkCharacterUIEntryPtr Entry, ESelectInfo::Type SelectionType) const;

	void OnPropertyChanged(const FPropertyChangedEvent& InEvent);

	void ConfigurationCharacter(const string& retargeterFilePath, const string& sampleFilePath, bool bAuto = false , unsigned short channel = 0, bool show = true);

	//Client --add by Ghost Chen
	//FLiveLinkClient* Client;

	TMap<ULiveLinkSourceFactory*, TSharedPtr<SWidget>> SourcePanels;

	// Handle to delegate registered with client so we can update when a source disappears
	FDelegateHandle OnSourcesChangedHandle;

	//Map to cover 
	TMap<UClass*, UObject*> DetailsPanelEditorObjects;

	TSharedPtr<SEditableTextBox> AddressPtr;
	TSharedPtr<SEditableTextBox> PortPtr;
	TSharedPtr<SEditableTextBox> SamplePathPtr;
	TSharedPtr<SEditableTextBox> RetargeterPathPtr;
	//Source list widget
	TSharedPtr<SListView<FLiveLinkCharacterUIEntryPtr>> ListView;
	//Source list items
	TArray<FLiveLinkCharacterUIEntryPtr> SourceData;
	//Command list items
	TSharedPtr<FUICommandList> CommandList;
	
	std::string SamplePath;
	std::string RetargeterPath;

	std::atomic<bool> Switch;
};