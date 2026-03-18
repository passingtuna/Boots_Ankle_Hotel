// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Types.h"
#include "Hotel_EventBase.generated.h"

class UEventInfo;
class UHotel_Manager;

UCLASS(Abstract)
class BOOTS_ANKLE_HOTEL_API UHotel_EventBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHotelEventId EventID = EHotelEventId::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFunctionExcuteTiming ExecuteTiming = FET_Init;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bEventOnlyOnce = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bExecutingOnlyEvent = false;

	virtual void Execute(UHotel_Manager* Manager, UEventInfo* EventInfo) PURE_VIRTUAL(UHotel_EventBase::Execute, );
	virtual bool CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName) PURE_VIRTUAL(UHotel_EventBase::CheckClear, return false;);
};

