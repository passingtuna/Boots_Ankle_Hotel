// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Types.h"
#include "Hotel_OutbreakEventBase.generated.h"

class UEventInfo;
class UHotel_Manager;

UCLASS(Abstract)
class BOOTS_ANKLE_HOTEL_API UHotel_OutbreakEventBase : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHotelOutbreakEventId OutbreakId = EHotelOutbreakEventId::None;

	// 아웃브레이크는 메뉴얼에 표시되는 ID가 0인 경우가 많아서 여기선 EventID를 따로 두지 않습니다.
	virtual void Execute(UHotel_Manager* Manager, UEventInfo* EventInfo) PURE_VIRTUAL(UHotel_OutbreakEventBase::Execute, );
	virtual bool CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger) PURE_VIRTUAL(UHotel_OutbreakEventBase::CheckClear, return false;);
};

