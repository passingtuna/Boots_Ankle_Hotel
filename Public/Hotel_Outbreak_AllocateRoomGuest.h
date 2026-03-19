// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_OutbreakEventBase.h"
#include "Hotel_Outbreak_AllocateRoomGuest.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_Outbreak_AllocateRoomGuest : public UHotel_OutbreakEventBase
{
	GENERATED_BODY()

public:
	UHotel_Outbreak_AllocateRoomGuest();

	virtual void Execute(UHotel_Manager* Manager, UEventInfo* EventInfo) override;
	virtual bool CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger) override;
};

