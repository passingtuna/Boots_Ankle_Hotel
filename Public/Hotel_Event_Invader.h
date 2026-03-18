// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_EventBase.h"
#include "Hotel_Event_Invader.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_Event_Invader : public UHotel_EventBase
{
	GENERATED_BODY()

public:
	UHotel_Event_Invader();

	virtual void Execute(UHotel_Manager* Manager, UEventInfo* EventInfo) override;
	virtual bool CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName) override;

private:
	static void TriggerInvade(UHotel_Manager* Manager);
};

