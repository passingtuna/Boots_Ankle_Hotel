// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_PersistenceService.h"

#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "HotelSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UHotel_PersistenceService::Init()
{
	MenualSaveName = TEXT("HotelManual.txt");

	// Default manual text
	UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/BluePrint/Data/Table/DT_Menual"));
	DefualtMenualText.Empty();
	if (Table)
	{
		static const FString Context(TEXT("DT_Menual"));
		TArray<FManualInfo*> TempRow;
		Table->GetAllRows<FManualInfo>(Context, TempRow);
		for (auto& Row : TempRow)
		{
			DefualtMenualText.Add(*Row);
		}
	}

	LoadGameLevelOption();
	if (!LoadGameManualExternal())
	{
		InitMenualInfo();
	}
}

void UHotel_PersistenceService::SaveLevelOption(int InMenualLevel, int InEnviromentLevel)
{
	EnviromentLevel = InEnviromentLevel;
	MenualLevel = InMenualLevel;
	SaveGameLevelOption();
}

void UHotel_PersistenceService::InitMenualInfo()
{
	arrExperiencedEventID.Empty();
	FioneerMenualText = TEXT("");
}

void UHotel_PersistenceService::UpdateDefualtLevelMenual(int EventId)
{
	if (MenualLevel != 1) return;
	if (EventId > 0 && !arrExperiencedEventID.Contains(EventId))
	{
		arrExperiencedEventID.Add(EventId);
		arrExperiencedEventID.Sort();
	}
	SaveGameLevelOption();
}

void UHotel_PersistenceService::SaveGameLevelOption()
{
	if (!HotelSaveGameOption) return;

	HotelSaveGameOption->MenualLevel = MenualLevel;
	HotelSaveGameOption->EnviromentLevel = EnviromentLevel;
	HotelSaveGameOption->arrExperiencedEventID = arrExperiencedEventID;

	if (IsValid(HotelSaveGameOption))
	{
		UGameplayStatics::SaveGameToSlot(HotelSaveGameOption, TEXT("PlayerSaveSlot"), 0);
	}
}

void UHotel_PersistenceService::LoadGameLevelOption()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("PlayerSaveSlot"), 0))
	{
		HotelSaveGameOption = Cast<UHotelSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PlayerSaveSlot"), 0));
		if (HotelSaveGameOption)
		{
			MenualLevel = HotelSaveGameOption->MenualLevel;
			EnviromentLevel = HotelSaveGameOption->EnviromentLevel;
			arrExperiencedEventID = HotelSaveGameOption->arrExperiencedEventID;
		}
	}
	else
	{
		HotelSaveGameOption = Cast<UHotelSaveGame>(UGameplayStatics::CreateSaveGameObject(UHotelSaveGame::StaticClass()));
		MenualLevel = 1;
		EnviromentLevel = 1;
		arrExperiencedEventID.Empty();
	}
}

bool UHotel_PersistenceService::LoadGameManualExternal()
{
	FString SaveDir = FPaths::ProjectSavedDir();
	FString FullPath = SaveDir / MenualSaveName;

	FString FileData;
	if (FFileHelper::LoadFileToString(FileData, *FullPath))
	{
		FioneerMenualText = FileData;
		return true;
	}
	return false;
}

bool UHotel_PersistenceService::SaveGameManualExternal()
{
	FString SaveDir = FPaths::ProjectSavedDir();
	FString FullPath = SaveDir / MenualSaveName;

	const FString OutputText = FioneerMenualText;
	return FFileHelper::SaveStringToFile(OutputText, *FullPath);
}

