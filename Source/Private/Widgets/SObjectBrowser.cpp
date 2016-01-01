// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "ObjectBrowserPrivatePCH.h"
#include "SObjectBrowser.h"
#include "SObjectBrowserTableRow.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "EditorFontGlyphs.h"

#include "ClassViewerModule.h"
#include "SClassPickerDialog.h"

#define LOCTEXT_NAMESPACE "SObjectBrowser"


/* SObjectBrowser interface
 *****************************************************************************/

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SObjectBrowser::Construct( const FArguments& InArgs )
{
	FilterClass = AActor::StaticClass();
	bShouldIncludeDefaultObjects = false;
	bOnlyListDefaultObjects = false;
	bOnlyListRootObjects = false;
	bOnlyListGCObjects = false;
	bIncludeTransient = true;

	// Create a property view
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	FDetailsViewArgs DetailsViewArgs(
		/*bUpdateFromSelection=*/ false,
		/*bLockable=*/ false,
		/*bAllowSearch=*/ true,
		FDetailsViewArgs::ObjectsUseNameArea,
		/*bHideSelectionTip=*/ true,
		/*InNotifyHook=*/ nullptr,
		/*InSearchInitialKeyFocus=*/ false,
		/*InViewIdentifier=*/ NAME_None);
	DetailsViewArgs.DefaultsOnlyVisibility = FDetailsViewArgs::EEditDefaultsOnlyNodeVisibility::Automatic;

	PropertyView = EditModule.CreateDetailView(DetailsViewArgs);

	ChildSlot
	[
		SNew(SSplitter)
		.Orientation(EOrientation::Orient_Horizontal)

		+ SSplitter::Slot()
		.Value(3)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.OnClicked(this, &SObjectBrowser::OnClassSelectionClicked)
						[
							SNew(STextBlock)
							.Text(this, &SObjectBrowser::GetFilterClassText)
						]
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						SNew(SEditableTextBox)
						.HintText(LOCTEXT("ObjectName", "Object Name"))
						.OnTextCommitted(this, &SObjectBrowser::OnNewHostTextCommited)
						.OnTextChanged(this, &SObjectBrowser::OnNewHostTextCommited, ETextCommit::Default)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SComboButton)
						.ComboButtonStyle(FEditorStyle::Get(), "ContentBrowser.Filters.Style")
						.ForegroundColor(FLinearColor::White)
						.ContentPadding(0)
						.ToolTipText(LOCTEXT("AddFilterToolTip", "Add a search filter."))
						.OnGetMenuContent(this, &SObjectBrowser::MakeFilterMenu)
						.HasDownArrow(true)
						.ContentPadding(FMargin(1, 0))
						.ButtonContent()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.Filters.Text")
								.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
								.Text(FEditorFontGlyphs::Filter)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(2, 0, 0, 0)
							[
								SNew(STextBlock)
								.TextStyle(FEditorStyle::Get(), "ContentBrowser.Filters.Text")
								.Text(LOCTEXT("Filters", "Filters"))
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.OnClicked(this, &SObjectBrowser::OnCollectGarbage)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CollectGarbage", "Collect Garbage"))
						]
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(FMargin(0.0f, 4.0f))
					[
						SAssignNew(ObjectListView, SListView< TSharedPtr<FBrowserObject> >)
						.ItemHeight(24.0f)
						.ListItemsSource(&LiveObjects)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SObjectBrowser::HandleListGenerateRow)
						.OnSelectionChanged(this, &SObjectBrowser::HandleListSelectionChanged)
						.HeaderRow
						(
							SNew(SHeaderRow)
							.Visibility(EVisibility::Collapsed)

							+ SHeaderRow::Column(NAME_Class)
							+ SHeaderRow::Column(NAME_Name)
							+ SHeaderRow::Column(NAME_Package)
						)
					]
				]
			]
		]

		+ SSplitter::Slot()
		.Value(2)
		[
			SNew(SBorder)
			.Padding(FMargin(3))
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				PropertyView.ToSharedRef()
			]
		]
	];

	RefreshList();
}

TSharedRef<SWidget> SObjectBrowser::MakeFilterMenu()
{
	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder MenuBuilder(bInShouldCloseWindowAfterMenuSelection, nullptr);

	MenuBuilder.BeginSection("AssetViewType", LOCTEXT("ViewTypeHeading", "View Type"));

	AddBoolFilter(
		MenuBuilder,
		LOCTEXT("ShouldIncludeDefaultObjects", "Include Default Objects (CDO)"),
		LOCTEXT("ShouldIncludeDefaultObjectsToolTip", "Should we include Class Default Objects in the results?"),
		&bShouldIncludeDefaultObjects);

	AddBoolFilter(
		MenuBuilder,
		LOCTEXT("OnlyListDefaultObjects", "Only List Default Objects"),
		LOCTEXT("OnlyListDefaultObjectsToolTip", ""),
		&bOnlyListDefaultObjects);

	AddBoolFilter(
		MenuBuilder,
		LOCTEXT("OnlyListRootObjects", "Only List Root Objects"),
		LOCTEXT("OnlyListRootObjectsToolTip", ""),
		&bOnlyListRootObjects);

	AddBoolFilter(
		MenuBuilder,
		LOCTEXT("OnlyListGCObjects", "Only List GC Objects"),
		LOCTEXT("OnlyListGCObjectsToolTip", ""),
		&bOnlyListGCObjects);

	AddBoolFilter(
		MenuBuilder,
		LOCTEXT("IncludeTransient", "Include Transient"),
		LOCTEXT("IncludeTransientToolTip", "Include objects in transient packages?"),
		&bIncludeTransient);

	return MenuBuilder.MakeWidget();
}

void SObjectBrowser::AddBoolFilter(FMenuBuilder& MenuBuilder, FText Text, FText ToolTip, bool* BoolOption)
{
	MenuBuilder.AddMenuEntry(
		Text,
		ToolTip,
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([=]{ *BoolOption = !( *BoolOption ); RefreshList(); }),
			FCanExecuteAction(),
			FIsActionChecked::CreateLambda([=]{ return *BoolOption; })
		),
		NAME_None,
		EUserInterfaceActionType::ToggleButton
	);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SObjectBrowser::RefreshList()
{
	UObject* CheckOuter = nullptr;
	UPackage* InsidePackage = nullptr;

	LiveObjects.Reset();

	for ( FObjectIterator It; It; ++It )
	{
		if ( It->IsTemplate(RF_ClassDefaultObject) )
		{
			if ( !bShouldIncludeDefaultObjects )
			{
				continue;
			}
		}
		else if ( bOnlyListDefaultObjects )
		{
			continue;
		}

		if ( bOnlyListGCObjects && GUObjectArray.IsDisregardForGC(*It) )
		{
			continue;
		}

		if ( bOnlyListRootObjects && !It->IsRooted() )
		{
			continue;
		}

		if ( CheckOuter && It->GetOuter() != CheckOuter )
		{
			continue;
		}

		if ( InsidePackage && !It->IsIn(InsidePackage) )
		{
			continue;
		}

		if ( !bIncludeTransient )
		{
			UPackage* ContainerPackage = It->GetOutermost();
			if ( ContainerPackage == GetTransientPackage() || ContainerPackage->HasAnyFlags(RF_Transient) )
			{
				continue;
			}
		}

		if ( FilterClass && !It->IsA(FilterClass) )
		{
			continue;
		}

		if ( !FilterString.IsEmpty() && !It->GetName().Contains(FilterString) )
		{
			continue;
		}

		TSharedPtr<FBrowserObject> NewObject = MakeShareable(new FBrowserObject());
		NewObject->Object = *It;

		LiveObjects.Add(NewObject);
	}

	ObjectListView->RequestListRefresh();
}

FReply SObjectBrowser::OnCollectGarbage()
{
	::CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	return FReply::Handled();
}


/* SObjectBrowser callbacks
 *****************************************************************************/

FText SObjectBrowser::GetFilterClassText() const
{
	if ( FilterClass )
	{
		return FilterClass->GetDisplayNameText();
	}

	return LOCTEXT("ClassFilter", "Class Filter");
}

FReply SObjectBrowser::OnClassSelectionClicked()
{
	const FText TitleText = LOCTEXT("PickClass", "Pick Class");
	
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, UObject::StaticClass());
	if ( bPressedOk )
	{
		FilterClass = ChosenClass;
		RefreshList();
	}

	return FReply::Handled();
}

void SObjectBrowser::OnNewHostTextCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	FilterText = InText;
	FilterString = FilterText.ToString();

	RefreshList();
}

TSharedRef<ITableRow> SObjectBrowser::HandleListGenerateRow(TSharedPtr<FBrowserObject> ObjectPtr, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SObjectBrowserTableRow, OwnerTable)
		.Object(ObjectPtr)
		.HighlightText(FilterText);
}

void SObjectBrowser::HandleListSelectionChanged(TSharedPtr<FBrowserObject> InItem, ESelectInfo::Type SelectInfo)
{
	if ( !InItem.IsValid() )
	{
		return;
	}

	TArray< TWeakObjectPtr<UObject> > Selection;
	Selection.Add(InItem->Object);
	
	PropertyView->SetObjects(Selection);
}

#undef LOCTEXT_NAMESPACE
