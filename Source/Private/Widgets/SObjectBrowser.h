// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widgets/SCompoundWidget.h"

struct FBrowserObject
{
public:
	TWeakObjectPtr<UObject> Object;
};

class IDetailsView;

/**
 * Implements the undo history panel.
 */
class SObjectBrowser : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SObjectBrowser) { }
	SLATE_END_ARGS()

public:

	/**
	 * Construct this widget
	 *
	 * @param InArgs The declaration data for this widget.
	 */
	void Construct( const FArguments& InArgs );

public:

private:

	TSharedRef<SWidget> MakeFilterMenu();

	void AddBoolFilter(FMenuBuilder& MenuBuilder, FText Text, FText ToolTip, bool* BoolOption);

	FReply OnCollectGarbage();

	void RefreshList();

	FText GetFilterClassText() const;

	FReply OnClassSelectionClicked();

	void OnNewHostTextCommited(const FText& InText, ETextCommit::Type InCommitType);

	TSharedRef<ITableRow> HandleListGenerateRow(TSharedPtr<FBrowserObject> TransactionInfo, const TSharedRef<STableViewBase>& OwnerTable);

	void HandleListSelectionChanged(TSharedPtr<FBrowserObject> TransactionInfo, ESelectInfo::Type SelectInfo);

private:

	FText FilterText;

	// Filters
	FString FilterString;
	UClass* FilterClass;
	bool bShouldIncludeDefaultObjects;
	bool bOnlyListDefaultObjects;
	bool bOnlyListRootObjects;
	bool bOnlyListGCObjects;
	bool bIncludeTransient;

	TArray< TSharedPtr<FBrowserObject> > LiveObjects;

	TSharedPtr< SListView< TSharedPtr<FBrowserObject> > > ObjectListView;

	TSharedPtr<IDetailsView> PropertyView;
};
