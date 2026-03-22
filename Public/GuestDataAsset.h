// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Hotel_Types.h"
#include "GuestDataAsset.generated.h"

/**
 * 
 */
class AHotel_Guest;
UCLASS()
class BOOTS_ANKLE_HOTEL_API UGuestDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AHotel_Guest> GuestClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool isMan = false;
};
