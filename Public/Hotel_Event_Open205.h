// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_EventBase.h"
#include "Hotel_Event_Open205.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_Event_Open205 : public UHotel_EventBase
{
	GENERATED_BODY()

public:
	UHotel_Event_Open205();

	virtual void Execute(UHotel_Manager* Manager, UEventInfo* EventInfo) override;
	virtual bool CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger) override;

private:
	static void Fail205(UHotel_Manager* Manager);
};

