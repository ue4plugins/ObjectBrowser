// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "ObjectBrowserPrivatePCH.h"
#include "ClassIconFinder.h"
#include "Toolkits/AssetEditorManager.h"
#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"

#define LOCTEXT_NAMESPACE "SObjectBrowserTableRow"


/**
 * Implements a row widget for a live uobject.
 */
class SObjectBrowserTableRow
	: public SMultiColumnTableRow<TSharedPtr<int32> >
{
public:

	SLATE_BEGIN_ARGS(SObjectBrowserTableRow) { }
		SLATE_ARGUMENT(TSharedPtr<FBrowserObject>, Object)
		SLATE_ARGUMENT(FText, HighlightText)
	SLATE_END_ARGS()

public:

	SObjectBrowserTableRow()
		: ClassIcon(nullptr)
	{
	}

	/**
	 * Constructs the widget.
	 *
	 * @param InArgs The construction arguments.
	 */
	void Construct( const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView )
	{
		BrowserObject = InArgs._Object;

		if ( UObject* Obj = InArgs._Object->Object.Get() )
		{
			Name = FText::FromString(Obj->GetName());
			ClassName = Obj->GetClass()->GetDisplayNameText();
			Package = FText::FromString(Obj->GetFullName());
			HighlightText = InArgs._HighlightText;

			// Get selection icon based on actor(s) classes and add before the selection label
			ClassIcon = FSlateIconFinder::FindIconForClass(Obj->GetClass()).GetIcon();
		}

		SMultiColumnTableRow<TSharedPtr<int32> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

public:

	virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override
	{
		if ( ColumnName == NAME_Class )
		{
			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SImage)
					.Image(ClassIcon)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
					.Text(ClassName)
				];
		}
		else if ( ColumnName == NAME_Name )
		{
			return SNew(STextBlock)
				.Text(Name)
				.HighlightText(HighlightText);
		}
		else if ( ColumnName == NAME_Package )
		{
			//return SNew(STextBlock)
			//	.Text(Package);
		}

		return SNullWidget::NullWidget;
	}

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		if ( UObject* LiveObject = BrowserObject->Object.Get() )
		{
			FAssetEditorManager::Get().OpenEditorForAsset(LiveObject);
		}

		return FReply::Handled();
	}

private:

	TSharedPtr<FBrowserObject> BrowserObject;

	const FSlateBrush* ClassIcon;

	// Holds the transaction's title text.
	FText Name;
	FText ClassName;
	FText Package;

	FText HighlightText;
};


#undef LOCTEXT_NAMESPACE
