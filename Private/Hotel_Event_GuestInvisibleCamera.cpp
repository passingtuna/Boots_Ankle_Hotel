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

	Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);

	if (AHotel_CCTV* CCTV = Manager->GetCCTV())
	{
		CCTV->SetHiddenObject(EventInfo->EventGuest);
	}
}

bool UHotel_Event_GuestInvisibleCamera::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;

	if (!EventInfo->EventGuest->GetAIController() || !EventInfo->EventGuest->GetAIController()->AssignedGuestRoom) return false;
	AHotel_Guest_Room* GuestRoom = EventInfo->EventGuest->GetAIController()->AssignedGuestRoom;
	const FString RoomNumber = GuestRoom->RoomNumber.ToString();

	if (FName(EventInfo->EventGuest->GuestName + "_In_" + RoomNumber) == TriggerName)
	{
		if (GuestRoom && GuestRoom->aDoor)
		{
			GuestRoom->aDoor->SetPeepingFace(true, false);
		}
	}

	if (FName(RoomNumber + "Door_OutPeepingEnd") == TriggerName)
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

