// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Hotel_Types.h"
#include "GameplayTagContainer.h"
#include "DA_EventInfo.generated.h"

/**
 * 
 */
class AHotel_Guest;
UCLASS()
class BOOTS_ANKLE_HOTEL_API UDA_EventFunctionInfo : public UPrimaryDataAsset
{
	GENERATED_BODY()

    int EventID;
    EFunctionExcuteTiming ExecuteTiming = FET_Init;
    bool isEventOnlyOnce = false;
    bool isExcutingOnlyEvent = false;//실행만하고 따로 트리거 체크를 하지않아 대기 리스트에 넣지 않음
    FGameplayTag FunctionTag;
};
