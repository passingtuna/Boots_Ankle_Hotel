// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Types.h"
#include "Hotel_PersistenceService.generated.h"

class UHotelSaveGame;

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_PersistenceService : public UObject
{
	GENERATED_BODY()

public:
	void Init();

	// ---- Options / progress ----
	int GetEnviromentalLevel() const { return EnviromentLevel; }
	int GetMenualLevel() const { return MenualLevel; }

	void SaveLevelOption(int InMenualLevel, int InEnviromentLevel);

	// ---- Manual / notes ----
	void SetFioneerMenaulText(const FString& Temp) { FioneerMenualText = Temp; }
	FString GetFioneerMenaulText() const { return FioneerMenualText; }

	TArray<FManualInfo>* GetMenualInfo() { return &DefualtMenualText; }
	TArray<int>* GetExperiencedEventID() { return &arrExperiencedEventID; }

	void InitMenualInfo();
	void UpdateDefualtLevelMenual(int EventId);

	// External text save for manual
	bool SaveGameManualExternal();
	bool LoadGameManualExternal();

	// SaveGame slot for options/progress
	void LoadGameLevelOption();
	void SaveGameLevelOption();

private:
	UPROPERTY()
	FString MenualSaveName;

	UPROPERTY()
	int EnviromentLevel = 1;

	UPROPERTY()
	int MenualLevel = 1;

	UPROPERTY()
	TArray<int> arrExperiencedEventID;

	UPROPERTY()
	TArray<FManualInfo> DefualtMenualText;

	UPROPERTY()
	UHotelSaveGame* HotelSaveGameOption = nullptr;

	UPROPERTY()
	FString FioneerMenualText;
};

