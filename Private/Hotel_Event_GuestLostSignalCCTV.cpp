// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Event_GuestLostSignalCCTV.h"

#include "Hotel_CCTV.h"
#include "Hotel_Guest.h"
#include "Hotel_Manager.h"

UHotel_Event_GuestLostSignalCCTV::UHotel_Event_GuestLostSignalCCTV()
{
	EventID = EHotelEventId::GuestLostSignalCCTV;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_GuestLostSignalCCTV::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

	UE_LOG(LogTemp, Warning, TEXT("로스트 시그널 게스트"));
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, true);
	if (IsValid(EventInfo->EventGuest))
	{
		EventInfo->EventGuest->IsWierdFaceLook = true;
		EventInfo->EventGuest->IsCheckTrigger = true;
	}
	Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
}

bool UHotel_Event_GuestLostSignalCCTV::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	AHotel_CCTV* CCTV = Manager->GetCCTV();
	if (!CCTV) return false;

	TArray<FString> Parts;
	TriggerName.ToString().ParseIntoArray(Parts, TEXT("_"), true);
	if (Parts.Num() < 3) return false;

	if (EventInfo->EventGuest->GuestName != Parts[0]) return false;

	if (Parts[1] == "In")
	{
		const int CameraNum = CCTV->FindCameraNumByName(FName(Parts[2]));
		if (CameraNum == -1) return false;
		CCTV->SetPostProcessCamera(CameraNum, 3, true);
	}
	else if (Parts[1] == "Out")
	{
		const int CameraNum = CCTV->FindCameraNumByName(FName(Parts[2]));
		if (CameraNum == -1) return false;
		CCTV->SetPostProcessCamera(CameraNum, 3, false);
	}

	return false;
}

