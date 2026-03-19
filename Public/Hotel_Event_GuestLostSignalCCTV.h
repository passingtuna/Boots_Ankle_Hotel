// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_EventBase.h"
#include "Hotel_Event_GuestLostSignalCCTV.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_Event_GuestLostSignalCCTV : public UHotel_EventBase
{
	GENERATED_BODY()

public:
	UHotel_Event_GuestLostSignalCCTV();

	virtual void Execute(UHotel_Manager* Manager, UEventInfo* EventInfo) override;
	virtual bool CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger) override;
};

