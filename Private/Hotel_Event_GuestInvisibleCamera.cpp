// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Event_GuestInvisibleCamera.h"

#include "Hotel_CCTV.h"
#include "Hotel_Door.h"
#include "Hotel_Guest.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"

UHotel_Event_GuestInvisibleCamera::UHotel_Event_GuestInvisibleCamera()
{
	EventID = EHotelEventId::GuestInvisibleCamera;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_GuestInvisibleCamera::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
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

	if (AHotel_CCTV* CCTV = Manager->GetCCTV())
	{
		CCTV->SetHiddenObject(EventInfo->EventGuest);
	}
}

bool UHotel_Event_GuestInvisibleCamera::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;

	if (!EventInfo->EventGuest->GetAIController() || !EventInfo->EventGuest->GetAIController()->AssignedGuestRoom) return false;
	AHotel_Guest_Room* GuestRoom = EventInfo->EventGuest->GetAIController()->AssignedGuestRoom;
	const FString RoomNumber = GuestRoom->RoomNumber.ToString();

	const FString* GuestNamePayload = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	const FString* PlacePayload = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* PlaceState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	if (Trigger.Type == EHotelTriggerType::PlaceStateChange
		&& GuestNamePayload && PlacePayload
		&& HotelTriggerStateEquals(PlaceState, EHotelObjectState::In)
		&& *GuestNamePayload == EventInfo->EventGuest->GuestName
		&& *PlacePayload == RoomNumber)
	{
		if (GuestRoom && GuestRoom->aDoor)
		{
			GuestRoom->aDoor->SetPeepingFace(true, false);
		}
	}

	const FString* PeepingRoom = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* PeepingState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	if (Trigger.Type == EHotelTriggerType::PeepingDoor
		&& PeepingRoom && PeepingState
		&& *PeepingRoom == RoomNumber
		&& HotelTriggerStateEquals(PeepingState, EHotelObjectState::End))
	{
		if (GuestRoom && GuestRoom->aDoor)
		{
			GuestRoom->aDoor->SetLockDoor(false);
			GuestRoom->aDoor->SetOpenDoor(true);
		}
		EventInfo->EventGuest->CatchingPlayer();
	}

	return false;
}

