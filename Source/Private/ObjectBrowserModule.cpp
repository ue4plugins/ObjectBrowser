// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ObjectBrowserModule.h"
#include "Widgets/SObjectBrowser.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "WorkspaceMenuStructure.h"
#include "EditorStyleSet.h"

#include "Editor/WorkspaceMenuStructure/Public/WorkspaceMenuStructureModule.h"
#include "SDockTab.h"

#define LOCTEXT_NAMESPACE "ObjectBrowser"

void FObjectBrowserModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ObjectBrowserTabName, FOnSpawnTab::CreateRaw(this, &FObjectBrowserModule::HandleSpawnSettingsTab))
		.SetDisplayName(LOCTEXT("ObjectBrowserTabTitle", "Object Browser"))
		.SetTooltipText(LOCTEXT("ObjectBrowserTooltipText", "Open the Object Browser tab."))
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "ObjectBrowser.TabIcon"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
}

void FObjectBrowserModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ObjectBrowserTabName);
}

bool FObjectBrowserModule::SupportsDynamicReloading()
{
	return true;
}

TSharedRef<SDockTab> FObjectBrowserModule::HandleSpawnSettingsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	const TSharedRef<SDockTab> DockTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab);

	DockTab->SetContent(SNew(SObjectBrowser));

	return DockTab;
}


IMPLEMENT_MODULE(FObjectBrowserModule, ObjectBrowser);


#undef LOCTEXT_NAMESPACE
