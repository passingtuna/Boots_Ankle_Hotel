// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Hotel_Manager.h"
#include "HotelSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotelSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    // 게임 옵션
    UPROPERTY(VisibleAnywhere, Category = "Options")
    int MenualLevel;

    UPROPERTY(VisibleAnywhere, Category = "Options")
    int EnviromentLevel;

    TArray<int> arrExperiencedEventID;
};
