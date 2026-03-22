// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Types.h"
#include "Hotel_EventBase.h"
#include "Hotel_OutbreakEventBase.generated.h"

class UEventInfo;
class UHotel_Manager;

UCLASS(Abstract)
class BOOTS_ANKLE_HOTEL_API UHotel_OutbreakEventBase : public UHotel_EventBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHotelOutbreakEventId OutbreakId = EHotelOutbreakEventId::None;
};

