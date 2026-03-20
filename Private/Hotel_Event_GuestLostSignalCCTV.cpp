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
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, true);
	if (IsValid(EventInfo->EventGuest))
	{
		EventInfo->EventGuest->IsWierdFaceLook = true;
		EventInfo->EventGuest->IsCheckTrigger = true;
	}
	Manager->UpdateDefualtLevelMenual(static_cast<int>(EventID));
}

bool UHotel_Event_GuestLostSignalCCTV::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	AHotel_CCTV* CCTV = Manager->GetCCTV();
	if (!CCTV) return false;

	const FString* Instigator = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	const FString* Place = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* PlaceState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	if (!Instigator || !Place || EventInfo->EventGuest->GuestName != *Instigator) return false;

	const int CameraNum = CCTV->FindCameraNumByName(FName(**Place));
	if (CameraNum == -1) return false;

	if (Trigger.Type == EHotelTriggerType::PlaceStateChange && HotelTriggerStateEquals(PlaceState, EHotelObjectState::In))
	{
		CCTV->SetPostProcessCamera(CameraNum, 3, true);
	}
	else if (Trigger.Type == EHotelTriggerType::PlaceStateChange && HotelTriggerStateEquals(PlaceState, EHotelObjectState::Out))
	{
		CCTV->SetPostProcessCamera(CameraNum, 3, false);
	}

	return false;
}

