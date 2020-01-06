// Fill out your copyright notice in the Description page of Project Settings.

#include "FgLiveLinkPanel.h"

#include "Editor.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SOverlay.h"
#include "Templates/SharedPointer.h"
#include "Widgets/Views/SListView.h"

#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/SlateDelegates.h"

#include "UObject/UObjectHash.h"
#include "EditorStyleSet.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IStructureDetailsView.h"
#include "Editor/EditorPerformanceSettings.h"

#include "Widgets/Notifications/SPopUpErrorText.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SSeparator.h"

#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"

#include "Misc/MessageDialog.h"
#include "XmlParser/Public/XmlParser.h"
#include "XmlParser/Public/XmlFile.h"
#include "XmlParser/Public/XmlNode.h"

#include "DesktopPlatform/Public/DesktopPlatformModule.h"

#include "FaceGood_LiveLinkStyle.h"
#include "FaceGood_LiveLinkCommands.h"

#include "FgLiveLinkSocket.h"
#include "FgControllerToBlendShape.h"
#include "GlobalObject.h"

#include <exception>
#include <filesystem>

#define LOCTEXT_NAMESPACE "LiveLinkPanel"

namespace fs = std::experimental::filesystem;

// Static Source UI FNames
namespace FgCharacterListUI
{
	static const FName CharacterColumnName(TEXT("Name"));
	static const FName ChannelColumnName(TEXT("Channel"));
	static const FName ShowColumnName(TEXT("Show"));
};

// Structure that defines a single entry in the source UI
struct FLiveLinkCharacterUIEntry
{
	friend class FgLiveLinkPanel;
public:
	FLiveLinkCharacterUIEntry(FGuid InEntryGuid, FText name, FText Channel, bool show)
		: EntryGuid(InEntryGuid),
		  Name(name),
		  Channel(Channel),
		  Show(show)
	{}

	FText GetCharacterName()	{ return Name;	  }
	FText GetCameraChannel()	{ return Channel;}
	bool GetCharacterStatus()	{ return Show;	  }

	FString GetRetargeterPath() { return RetargeterPath;}
	FString GetSamplePath()		{ return SampingPath; }

	void RemoveFromClient() { /*Client->RemoveSource(EntryGuid);*/ }
	void OnPropertyChanged(const FPropertyChangedEvent& InEvent) { /*Client->OnPropertyChanged(EntryGuid, InEvent);*/ }

private:
	FGuid EntryGuid;
	FText Name;
	FText Channel;
	bool Show;

	FString SampingPath;
	FString RetargeterPath;

	TSharedPtr<FgControllerToBlendShape> Controller;
};

class SLiveLinkCharacterPanelSourcesRow : public SMultiColumnTableRow<FLiveLinkCharacterUIEntryPtr>
{
public:
	SLATE_BEGIN_ARGS(SLiveLinkCharacterPanelSourcesRow) {}

	/** The list item for this row */
	SLATE_ARGUMENT(FLiveLinkCharacterUIEntryPtr, Entry)

	SLATE_END_ARGS()

	void Construct(const FArguments& Args, const TSharedRef<STableViewBase>& OwnerTableView)
	{
		EntryPtr = Args._Entry;

		CharacterName = EntryPtr->GetCharacterName();
		Channel		  = EntryPtr->GetCameraChannel();
		Show =			EntryPtr->GetCharacterStatus();

		SMultiColumnTableRow<FLiveLinkCharacterUIEntryPtr>::Construct
		(
			FSuperRowType::FArguments()
			.Padding(1.0f),
			OwnerTableView
		);
	}

	/** Overridden from SMultiColumnTableRow.  Generates a widget for this column of the list view. */
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (ColumnName == FgCharacterListUI::CharacterColumnName)
			return	SNew(STextBlock).Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLiveLinkCharacterPanelSourcesRow::GetCharacterName)));
		else if (ColumnName == FgCharacterListUI::ChannelColumnName)
			return	SNew(SEditableTextBox).Text(TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SLiveLinkCharacterPanelSourcesRow::GetCameraChannel)));
		else if (ColumnName == FgCharacterListUI::ShowColumnName)
			return	SNew(SCheckBox).IsFocusable(TAttribute<bool>::Create(TAttribute<bool>::FGetter::CreateSP(this, &SLiveLinkCharacterPanelSourcesRow::GetCharacterStatus)).Get());

		return SNullWidget::NullWidget;
	}

private:
	FText GetCharacterName() const { return CharacterName;}
	FText GetCameraChannel() const { return Channel;}
	bool GetCharacterStatus()const { return Show;}

	FLiveLinkCharacterUIEntryPtr EntryPtr;

	FText CharacterName;
	FText Channel;
	bool Show;
};

FgLiveLinkPanel::~FgLiveLinkPanel()
{
	/*if (Client)
	{
		Client->UnregisterSourcesChangedHandle(OnSourcesChangedHandle);
		OnSourcesChangedHandle.Reset();
	}*/
}

void FgLiveLinkPanel::Construct(const FArguments& Args/*, FLiveLinkClient* InClient*/)
{
	/*check(InClient);
	Client = InClient;*/

	Switch = true;

	CommandList = MakeShareable(new FUICommandList);

	BindCommands();

	FToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None);

	ToolBarBuilder.BeginSection(TEXT("Add"));
	{
		ToolBarBuilder.AddToolBarButton(FFaceGood_LiveLinkCommands::Get().RemoveSource, 
										NAME_None,
									    LOCTEXT("Remove Character", "Remove Selected Item"),
									    LOCTEXT("RemoveSource_ToolTip", "Remove selected Characters"),
									    FSlateIcon(FName("FgLiveLinkStyle"), "LiveLink.Common.RemoveSource")
										);
		ToolBarBuilder.AddToolBarButton(FFaceGood_LiveLinkCommands::Get().RemoveAllSources,
										NAME_None,
										LOCTEXT("Remove All Characters", "Remove All Items"),
										LOCTEXT("RemoveAllSource_ToolTip", "Remove All Live Link Characters"),
										FSlateIcon(FName("FgLiveLinkStyle"), "LiveLink.Common.RemoveAllSources")
										);
		ToolBarBuilder.AddSeparator();
		ToolBarBuilder.AddToolBarButton(FFaceGood_LiveLinkCommands::Get().ImportAllSource,
										NAME_None,
										LOCTEXT("Import Character", "Import Character(s)"),
										LOCTEXT("ImportSource_ToolTip", "Import All Live Link Characters from xml file"),
										FSlateIcon(FName("FgLiveLinkStyle"), "LiveLink.Common.ImportAllSource")
										);
		ToolBarBuilder.AddToolBarButton(FFaceGood_LiveLinkCommands::Get().ExportAllSources,
										NAME_None,
										LOCTEXT("Export All Characters", "Export Characters"),
										LOCTEXT("ExportSource_ToolTip",  "Export All Live Link Characters to xml file"),
										FSlateIcon(FName("FgLiveLinkStyle"), "LiveLink.Common.ExportAllSources")
										);
	}
	ToolBarBuilder.EndSection();

	// Connection Settings
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs;
	//DetailsViewArgs.
	FStructureDetailsViewArgs StructureViewArgs;
	StructureViewArgs.bShowAssets = true;
	StructureViewArgs.bShowClasses = true;
	StructureViewArgs.bShowInterfaces = true;
	StructureViewArgs.bShowObjects = true;

	const int WarningPadding = 8;

	UProperty* PerformanceThrottlingProperty = FindFieldChecked<UProperty>(UEditorPerformanceSettings::StaticClass(), GET_MEMBER_NAME_CHECKED(UEditorPerformanceSettings, bThrottleCPUWhenNotForeground));
	PerformanceThrottlingProperty->GetDisplayNameText();
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("PropertyName"), PerformanceThrottlingProperty->GetDisplayNameText());
	FText PerformanceWarningText = FText::Format(LOCTEXT("LiveLinkPerformanceWarningMessage", "Warning: The editor setting '{PropertyName}' is currently enabled\nThis will stop editor windows from updating in realtime while the editor is not in focus"), Arguments);

	/*TSharedRef<SScrollBar> ExternalScrollbar =
		SNew(SScrollBar)
		.AlwaysShowScrollbar(DetailsViewArgs.bShowScrollBar)
		.Visibility(EVisibility::Visible);*/

	//UI Create Section
	ChildSlot
	[	
		SNew(SVerticalBox)

		//NetWork Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
		[
			SNew(SExpandableArea)
			.AreaTitle(FText::FromString(FString("Network")))
			.InitiallyCollapsed(false)
			.ToolTipText(FText::FromString(FString("Establishing Data Transmission with FaceGood Streamer")))
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.45f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("Address:")))
					]

					+ SSplitter::Slot()
					.Value(0.45f)
					[
						SAssignNew(AddressPtr, SEditableTextBox)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("127.0.0.1")))
					]
				]

				/*+ SVerticalBox::Slot()
				[
					SNew(SSeparator)
					.Thickness(2.0f)
				]*/

				+ SVerticalBox::Slot()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("Port:")))
					]

				+ SSplitter::Slot()
					.Value(0.5f)
					[
						SAssignNew(PortPtr, SEditableTextBox)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("43012")))
					]
				]

				+ SVerticalBox::Slot()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("Connect:")))
					]

					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SCheckBox)
						.HAlign(HAlign_Center)
						.OnCheckStateChanged(this, &FgLiveLinkPanel::NetStateChanged)
						.UncheckedImage(FFaceGood_LiveLinkStyle::Get().GetBrush(FName("FgLiveLinkStyle"), "LiveLink.Common.RemoveAllSources"))
						.CheckedImage(FFaceGood_LiveLinkStyle::Get().GetBrush(FName("FgLiveLinkStyle"), "LiveLink.Common.RemoveSource"))
					]
				]
			]
		]

		//Character Manage Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
		[
			SNew(SExpandableArea)
			.AreaTitle(FText::FromString(FString("Manage")))
			.InitiallyCollapsed(false)
			.BodyContent()
			[
				ToolBarBuilder.MakeWidget()
			]
		]

		//Confiure Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
		[
			SNew(SExpandableArea)
			.AreaTitle(FText::FromString(FString("Confiure")))
			.InitiallyCollapsed(false)
			.BodyContent()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SSplitter)
						.Orientation(Orient_Horizontal)

						+ SSplitter::Slot()
						.Value(0.33f)
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(FText::FromString(FString("Sampling:")))
						]

						+ SSplitter::Slot()
						.Value(0.33f)
						[
							SAssignNew(SamplePathPtr, SEditableTextBox)
							.IsReadOnly(true)
							.Text(FText::FromString(FString("")))
						]

						+ SSplitter::Slot()
						.Value(0.33f)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.IsFocusable(true)
							.Text(FText::FromString(FString("Browse")))
							.IsFocusable(true)
							.OnClicked(this, &FgLiveLinkPanel::OnClickBrowseSamping)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.33f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("Retargeter:")))
					]

					+ SSplitter::Slot()
					.Value(0.33f)
					[
						SAssignNew(RetargeterPathPtr, SEditableTextBox)
						.IsReadOnly(true)
						.Text(FText::FromString(FString("")))
					]

					+ SSplitter::Slot()
					.Value(0.33f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString(FString("Browse")))
						.IsFocusable(true)
						.OnClicked(this, &FgLiveLinkPanel::OnClickBrowseRetargeter)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.f)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(EHorizontalAlignment::HAlign_Right)
					[
						SNew(SButton)
						.Text(LOCTEXT("OkButton", "Configure"))
						.OnClicked(this, &FgLiveLinkPanel::OnClickConfigure)
					]
				]
			]
		]

		//Character List View Section
		+ SVerticalBox::Slot()
		//.FillHeight(0.5f)
		.AutoHeight()
		.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
		[
			SNew(SExpandableArea)
			.AreaTitle(FText::FromString(FString("Character List")))
			.InitiallyCollapsed(false)
			.BodyContent()
			[
				SNew(SSplitter)
				.Orientation(EOrientation::Orient_Horizontal)
				+ SSplitter::Slot()
				.Value(0.5f)
				[
					SNew(SSplitter)
					.Orientation(EOrientation::Orient_Vertical)
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SBorder)
						.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
						.Padding(FMargin(4.0f, 4.0f))
						[
							SAssignNew(ListView, SListView<FLiveLinkCharacterUIEntryPtr>)
							.ListItemsSource(&SourceData)
							.SelectionMode(ESelectionMode::SingleToggle)
							.ItemHeight(24)
							.OnGenerateRow(this, &FgLiveLinkPanel::MakeSourceListViewWidget)
							.OnSelectionChanged(this, &FgLiveLinkPanel::OnSourceListSelectionChanged)
							//.ExternalScrollbar(ExternalScrollbar)
							.HeaderRow
							(
								SNew(SHeaderRow)
								+ SHeaderRow::Column(FgCharacterListUI::CharacterColumnName)
								.FillWidth(20.0f)
								.DefaultLabel(LOCTEXT("CharacterColumnHeaderName", "Character Name"))

								+ SHeaderRow::Column(FgCharacterListUI::ChannelColumnName)
								.FillWidth(10.0f)
								.DefaultLabel(LOCTEXT("ChannelColumnHeaderName", "Camera Channel"))

								+ SHeaderRow::Column(FgCharacterListUI::ShowColumnName)
								.FillWidth(10.0f)
								.DefaultLabel(LOCTEXT("StatusColumnHeaderName", "Character Show"))
							)
						]
					]
				]
			]
		]

		//Support Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
		[
			SNew(SExpandableArea)
			.AreaTitle(FText::FromString(FString("Support")))
			.InitiallyCollapsed(false)
			.ToolTipText(FText::FromString(FString("Technical support")))
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("UserGuide:")))
					]
					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString(FString("QQ::794569465")))
						.ButtonColorAndOpacity(FColor::Orange)
					]
				]
				+ SVerticalBox::Slot()
				[
					SNew(SSplitter)
					.Orientation(Orient_Horizontal)

					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(STextBlock)
						.Justification(ETextJustify::Center)
						.Text(FText::FromString(FString("HomePage:")))
					]

					+ SSplitter::Slot()
					.Value(0.5f)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.Text(FText::FromString(FString("www.facegood.cc")))
						.OnClicked(this, &FgLiveLinkPanel::OnClickHomePage)
						.ButtonColorAndOpacity(FColor::Orange)
					]
				]
			]
		]

		//Warning Section
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("SettingsEditor.CheckoutWarningBorder"))
			.BorderBackgroundColor(FColor(166, 137, 0))
			.Visibility(this, &FgLiveLinkPanel::ShowEditorPerformanceThrottlingWarning)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(FMargin(WarningPadding, WarningPadding, WarningPadding, WarningPadding))
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(PerformanceWarningText)
					.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.ShadowColorAndOpacity(FLinearColor::Black.CopyWithNewOpacity(0.3f))
					.ShadowOffset(FVector2D::UnitVector)
				]

				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, WarningPadding, 0.f))
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.OnClicked(this, &FgLiveLinkPanel::DisableEditorPerformanceThrottling)
					.Text(LOCTEXT("LiveLinkPerformanceWarningDisable", "Disable"))
				]
			]
		]
	];
}

void FgLiveLinkPanel::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObjects(DetailsPanelEditorObjects);
}

void FgLiveLinkPanel::BindCommands()
{
	CommandList->MapAction(FFaceGood_LiveLinkCommands::Get().RemoveSource,
						   FExecuteAction::CreateSP(this, &FgLiveLinkPanel::HandleRemoveSource));

	CommandList->MapAction(FFaceGood_LiveLinkCommands::Get().RemoveAllSources,
						   FExecuteAction::CreateSP(this, &FgLiveLinkPanel::HandleRemoveAllSources));

	CommandList->MapAction(FFaceGood_LiveLinkCommands::Get().ImportAllSource,
						   FExecuteAction::CreateSP(this, &FgLiveLinkPanel::HandleImportSource));

	CommandList->MapAction(FFaceGood_LiveLinkCommands::Get().ExportAllSources,
						   FExecuteAction::CreateSP(this, &FgLiveLinkPanel::HandleExportSource));
}

const TArray<FLiveLinkCharacterUIEntryPtr>& FgLiveLinkPanel::GetCurrentSources() const
{
	return SourceData;
}

TSharedRef<ITableRow> FgLiveLinkPanel::MakeSourceListViewWidget(FLiveLinkCharacterUIEntryPtr Entry, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(SLiveLinkCharacterPanelSourcesRow, OwnerTable).Entry(Entry);
}

void FgLiveLinkPanel::OnSourceListSelectionChanged(FLiveLinkCharacterUIEntryPtr Entry, ESelectInfo::Type SelectionType) const
{
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("OnSourceListSelectionChanged!")));
}

void FgLiveLinkPanel::OnPropertyChanged(const FPropertyChangedEvent& InEvent)
{
	TArray<FLiveLinkCharacterUIEntryPtr> Selected;
	ListView->GetSelectedItems(Selected);
	for (const FLiveLinkCharacterUIEntryPtr& Item : Selected)
	{
		Item->OnPropertyChanged(InEvent);
	}
}

void FgLiveLinkPanel::NetStateChanged(ECheckBoxState InState)
{
	switch (InState)
	{
		case ECheckBoxState::Checked :
		{
			//Connect
			if (FgLiveLinkSocket::connect(AddressPtr->GetText().ToString(), FCString::Atoi(*PortPtr->GetText().ToString())))
			{
				static int invoke = 1;
				if (invoke++ == 1)
				{
					AnimationSwitch(true);
					std::thread tr(&FgLiveLinkPanel::AnimationEngine, this);
					tr.detach();
				}
			}
			else
			{
				AnimationSwitch(false);
				FgLiveLinkSocket::disconnect();
			}
			break;
		}
		case ECheckBoxState::Unchecked:
		{
			//Disconnect
			FgLiveLinkSocket::disconnect();
			break;
		}
		default:
		{
			break;
		}
	}
}

void FgLiveLinkPanel::HandleRemoveSource()
{
	TArray<FLiveLinkCharacterUIEntryPtr> Selected;
	ListView->GetSelectedItems(Selected);
	if (Selected.Num() > 0) SourceData.RemoveSingle(Selected[0]);

	ListView->RequestListRefresh();
}

void FgLiveLinkPanel::HandleRemoveAllSources()
{
	SourceData.Reset(0);			//May Bug Occur
	ListView->RequestListRefresh();
}

void FgLiveLinkPanel::HandleImportSource()
{
	TArray<FString> TempFile;
	if (!FDesktopPlatformModule::Get()->OpenFileDialog(NULL, TEXT("Choose character's configure file"), FPaths::GetRelativePathToRoot(), TEXT(""), "xml file|*.xml", EFileDialogFlags::None, TempFile))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("Failured to import retargeter file!")));
		return;
	}

	FString strPath = TempFile[0];

	try
	{
		FXmlFile xmlParser;
		xmlParser.LoadFile(strPath, EConstructMethod::ConstructFromFile);

		//Root Node
		FXmlNode* rootNode = xmlParser.GetRootNode();

		//NetWork Node
		FXmlNode* networkNode = rootNode->FindChildNode("network");
		FString address = networkNode->GetAttribute("address");
		FString port = networkNode->GetAttribute("port");
		FString number = networkNode->GetAttribute("number");

		//configuration Node
		FXmlNode* configurationNode = rootNode->FindChildNode("configuration");
		const TArray<FXmlNode*> childNodes = configurationNode->GetChildrenNodes();
		for (const auto& node : childNodes)
		{
			FString name = node->GetAttribute("name");
			string retargeterPath = std::string(TCHAR_TO_UTF8(*node->GetAttribute("retargeterPath")));
			string samplePath = std::string(TCHAR_TO_UTF8(*node->GetAttribute("samplePath")));
			int channel = FCString::Atoi(*node->GetContent());

			ConfigurationCharacter(retargeterPath, samplePath, true, channel);
		}
	}
	catch (...)
	{
		FMessageDialog::ShowLastError();
		return;
	}
}

void FgLiveLinkPanel::HandleExportSource()
{
	if (SourceData.GetAllocatedSize() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("No Character!")));
		return;
	}

	TArray<FString> TempFile;
	if (!FDesktopPlatformModule::Get()->SaveFileDialog(NULL, "Save Current Characters to xml File", FPaths::GetProjectFilePath(), "Characters.xml", "xml file|*.xml", EFileDialogFlags::None, TempFile))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("No Character!")));
		return;
	}
											
	FString strXmlSavePath = TempFile[0];

	try
	{
		string xmlContent = "<root version=\"1.0\">\n";

		string address = std::string(TCHAR_TO_UTF8(*AddressPtr->GetText().ToString()));
		string port = std::string(TCHAR_TO_UTF8(*PortPtr->GetText().ToString()));
		string rate = "500.00";
		string number = std::to_string(SourceData.GetAllocatedSize());

		xmlContent += string("<network address=\"") + address;
		xmlContent += string("\" port=\"")			+ port;
		xmlContent += string("\" rate=\"")			+ rate;
		xmlContent += string("\" number=\"")		+ number  + string("\">1</network>\n");
		xmlContent += string("<configuration>\n");
		for (const auto& Character : SourceData)
		{
			string name		  = std::string(TCHAR_TO_UTF8(*Character->GetCharacterName().ToString()));
			string retargeter = std::string(TCHAR_TO_UTF8(*Character->GetRetargeterPath()));
			string sampling   = std::string(TCHAR_TO_UTF8(*Character->GetSamplePath()));
			string channel    = std::string(TCHAR_TO_UTF8(*Character->GetCameraChannel().ToString()));
			xmlContent += "<Character name=\""	 + name;
			xmlContent += "\" retargeterPath=\"" + retargeter;
			xmlContent += "\" samplePath=\""	 + sampling   + "\">";
			xmlContent += channel;
			xmlContent += "</Character>\n";
		}
		xmlContent += string("</configuration>\n");
		xmlContent += string("</root>");

		FString content = xmlContent.c_str();

		FXmlFile xmlParser(content, EConstructMethod::ConstructFromBuffer);
		xmlParser.Save(strXmlSavePath);
	}
	catch (...)
	{
		FMessageDialog::ShowLastError();
		return;
	}
}

void FgLiveLinkPanel::AnimationSwitch(bool open)
{
	Switch = open;
}

void FgLiveLinkPanel::AnimationEngine()
{
	while (true)
	{
		while (Switch)
		{
			vector<float>* weights = nullptr;
			weights = FaceGood_LiveLink::IOSFacialDataFloatVec.Pop();
			if ((weights != nullptr) && (weights->size() > 0))
			{
				vector<float> tempWeight;
				tempWeight = *weights;

				//Bug here
				int identifier = std::round(tempWeight.back());
				tempWeight.pop_back();
				for (const auto& character : SourceData)
				{
					if (!FaceGood_LiveLink::RunFlag) return;

					//Need to check here 
					characterLabel label = character->Controller->getCharacter();
					int channel = label.channel;
					if (channel == identifier)
					{
						if (bool use = label.use) character->Controller->EvaluateComponentSpaceInternal(tempWeight);
					}
				}
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

EVisibility FgLiveLinkPanel::ShowEditorPerformanceThrottlingWarning() const
{
	const UEditorPerformanceSettings* Settings = GetDefault<UEditorPerformanceSettings>();
	return /*Settings->bThrottleCPUWhenNotForeground ? */EVisibility::Visible/* : EVisibility::Collapsed*/;
}

FReply FgLiveLinkPanel::DisableEditorPerformanceThrottling()
{
	UEditorPerformanceSettings* Settings = GetMutableDefault<UEditorPerformanceSettings>();
	Settings->bThrottleCPUWhenNotForeground = false;
	Settings->PostEditChange();
	Settings->SaveConfig();
	return FReply::Handled();
}

FReply FgLiveLinkPanel::OnClickHomePage()
{
	FString TheURL = "www.facegood.cc";
	FPlatformProcess::LaunchURL(*TheURL, nullptr, nullptr);
	return FReply::Handled();
}

FReply FgLiveLinkPanel::OnClickBrowseSamping()
{
	FString outFolderName = "";

	if (!FDesktopPlatformModule::Get()->OpenDirectoryDialog(NULL, TEXT("choose Samping Path"), FPaths::GetProjectFilePath(), outFolderName))
	{
		FText title = FText::FromString("Error");
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("Falired to get Sampling path")), &title);
		SamplePathPtr->SetText(FText::FromString(FString("")));
		SamplePath = "";
		return FReply::Unhandled();
	}
	SamplePath = TCHAR_TO_UTF8(*outFolderName);
	fs::path Path = SamplePath;

	SamplePathPtr->SetText(FText::FromString(Path.filename().c_str()));

	FSlateApplication::Get().DismissAllMenus();
	return FReply::Handled();
}

FReply FgLiveLinkPanel::OnClickBrowseRetargeter()
{
	TArray<FString> OpenFilenames;

	if (!FDesktopPlatformModule::Get()->OpenFileDialog(NULL, TEXT("Choose Retargeter file"), FPaths::GetRelativePathToRoot(), TEXT(""), "retargeter file|*.retargeter", EFileDialogFlags::None, OpenFilenames))
	{
		FText title = FText::FromString("Error");
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("Falired to get Retargeter path")), &title);
		RetargeterPathPtr->SetText(FText::FromString(FString("")));
		RetargeterPath = "";
		return FReply::Unhandled();
	}
	RetargeterPath = TCHAR_TO_UTF8(*OpenFilenames[0]);
	fs::path Path = RetargeterPath;
	RetargeterPathPtr->SetText(FText::FromString(Path.filename().c_str()));
	
	FSlateApplication::Get().DismissAllMenus();
	return FReply::Handled();
}

void FgLiveLinkPanel::ConfigurationCharacter(const string& retargeterFilePath, const string& sampleFilePath, bool bAuto, unsigned short channel, bool show)
{
	if (!fs::exists(sampleFilePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("The current \"Sampling\" Path not exist,check first!")));
		return;
	}

	if (!fs::exists(retargeterFilePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("The current \"retargeter\" file not exist,check first!")));
		return;
	}

	TSharedPtr<FLiveLinkCharacterUIEntry> rowItem = MakeShareable(new FLiveLinkCharacterUIEntry(FGuid::NewGuid(), FText::FromString(fs::path(retargeterFilePath).filename().replace_extension().c_str()), FText::FromString(std::to_string(channel).c_str()), show));

	//Aquire the Imformation
	std::list<std::vector<float>> totalControllerValue;
	std::list<string> TempListExpression;
	vector<int> vectorKeyNum, vectorContrllerNum;
	int totalKeyCount, totalControllerCount;
	totalKeyCount = 0; totalControllerCount = 0;

	rowItem->Controller = MakeShared<FgControllerToBlendShape>();

	try
	{
		FXmlFile xmlParser(retargeterFilePath.c_str(), EConstructMethod::ConstructFromFile);
		FXmlNode* RootNode = xmlParser.GetRootNode();
		int totalGroup = FCString::Atoi(*RootNode->FindChildNode("head")->FindChildNode("TotalGroup")->GetContent());

		const TArray<FXmlNode*> childNodes = RootNode->GetChildrenNodes();
		for (const auto& node : childNodes)
		{
			if (node->GetTag() == "group")
			{
				int keyframeNum = FCString::Atoi(*node->FindChildNode("KeyFrames")->GetContent());
				totalKeyCount += keyframeNum;
				vectorKeyNum.push_back(keyframeNum);

				int controllerNum = FCString::Atoi(*node->FindChildNode("ControllerNum")->GetContent());
				totalControllerCount += controllerNum;
				vectorContrllerNum.push_back(controllerNum);

				string stringControllerName, temp;
				stringControllerName = std::string(TCHAR_TO_UTF8(*node->FindChildNode("ControllerName")->GetContent()));
				stringControllerName.erase(std::remove(stringControllerName.begin(), stringControllerName.end(), '\t'), stringControllerName.end());
				stringControllerName.erase(std::remove(stringControllerName.begin(), stringControllerName.end(), '\n'), stringControllerName.end());
				stringControllerName.erase(std::remove(stringControllerName.begin(), stringControllerName.end(), '\0'), stringControllerName.end());
				stringControllerName.erase(std::remove(stringControllerName.begin(), stringControllerName.end(), ' '), stringControllerName.end());
				stringstream input(stringControllerName);
				while (std::getline(input, temp, ','))	rowItem->Controller->mVectorString.push_back(FName((string("FACEGOOD_") + temp).c_str()));

				const TArray<FXmlNode*> DataNode = node->FindChildNode("ControllerData")->GetChildrenNodes();
				for (const auto& iter : DataNode)
				{
					std::string stringData, temp1;
					std::vector<float> tempVecFloat;

					TempListExpression.push_back(std::string(TCHAR_TO_UTF8(*iter->GetAttribute("remark"))));

					stringData = std::string(TCHAR_TO_UTF8(*iter->GetContent()));
					stringstream Data(stringData);
					while (std::getline(Data, temp1, ','))
					{
						float fvalue = atof(temp1.c_str());
						tempVecFloat.push_back(fvalue);
					}

					totalControllerValue.push_back(tempVecFloat);
				}
			}
		}

		rowItem->Controller->mIOSFacialBlendShapeVec.clear();
		rowItem->Controller->mIOSFacialBlendShapeVec.resize(totalKeyCount);
		rowItem->Controller->mListExpression = TempListExpression;

		int front, self, back;
		int iter, index, mark;
		front = 0;
		self = vectorContrllerNum[0];
		back = totalControllerCount - self;
		iter = index = mark = 0;
		for (auto& LIST : totalControllerValue)
		{
			vector<float> frontTemp(front, 0.0f);
			vector<float> backTemp(back, 0.0f);
			vector<float> Temp;

			Temp.insert(Temp.end(), frontTemp.begin(), frontTemp.end());
			Temp.insert(Temp.end(), LIST.begin(), LIST.end());
			Temp.insert(Temp.end(), backTemp.begin(), backTemp.end());

			rowItem->Controller->mIOSFacialBlendShapeVec[index++] = Temp;

			++mark;

			if (mark == vectorKeyNum[iter])
			{
				++iter;
				self = vectorContrllerNum[iter];
				front += vectorContrllerNum[iter - 1];
				back = totalControllerCount - self - front;

				mark = 0;
			}
		}

		rowItem->Controller->m_label.name = fs::path(retargeterFilePath).filename().replace_extension().string();

		if (!rowItem->Controller->Init(sampleFilePath, false))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString("Error occured in the configuration file which has defected\nPlease check the file's Integrity first!")));
			//mDevice->AnimationStart();
			return;
		}

		if (!bAuto)
		{
			//judge
			unsigned short identifier = 0;
			bool FindFlag = false;
			for (const auto& Character : SourceData)
			{
				unsigned short ID = Character->Controller->m_label.channel;
				if (ID >= channel)
				{
					identifier = ID;
					FindFlag = true;
				}
			}
			if (FindFlag)
			{
				identifier += 1;
			}

			channel = identifier;
		}

		int currentCharacterNum = SourceData.GetAllocatedSize();

		rowItem->Controller->m_label.channel = channel;
		rowItem->Controller->m_label.use = true;

		//mListWhich.Items.Add(modelName.c_str());
	}
	catch (std::exception& except)
	{
		FText title = FText::FromString("Error");
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(FString(except.what())), &title);
	}

	SourceData.Add(rowItem);
	ListView->RequestListRefresh();
}

FReply FgLiveLinkPanel::OnClickConfigure()
{
	ConfigurationCharacter(RetargeterPath, SamplePath);

	FSlateApplication::Get().DismissAllMenus();
	return FReply::Handled();
}
