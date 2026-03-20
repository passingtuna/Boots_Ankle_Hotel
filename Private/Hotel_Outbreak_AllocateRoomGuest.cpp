// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Outbreak_AllocateRoomGuest.h"

#include "Hotel_Guest.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"
#include "Hotel_Operator.h"
#include "Hotel_Phone.h"

UHotel_Outbreak_AllocateRoomGuest::UHotel_Outbreak_AllocateRoomGuest()
{
	OutbreakId = EHotelOutbreakEventId::AllocateRoomGuest;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Outbreak_AllocateRoomGuest::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;
	UWorld* World = Manager->GetWorld();
	if (!World) return;

	// 빈 방 목록 수집 (205/305 제외)
	TArray<FString> EmptyRooms;
	const TMap<FName, FRoomInfo>& Rooms = Manager->GetRegisteredRooms();
	for (const auto& Kvp : Rooms)
	{
		if (!IsValid(Kvp.Value.RoomGuest) && Kvp.Value.Room)
		{
			const FName RoomNum = Kvp.Value.Room->RoomNumber;
			if (RoomNum != "205" && RoomNum != "305")
			{
				EmptyRooms.Add(Kvp.Key.ToString());
			}
		}
	}
	if (EmptyRooms.IsEmpty()) return;

	const FString AllocatedRoom = EmptyRooms[FMath::RandRange(0, EmptyRooms.Num() - 1)];
	EventInfo->CollectedTriggers.Add(
		FHotelTrigger::Make(
			EHotelTriggerType::RoomAssigned,
			{
				{ EHotelTriggerKey::Target, AllocatedRoom },
			}));

	TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
	TWeakObjectPtr<UEventInfo> WeakEvent(EventInfo);
	World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, WeakEvent]()
		{
			if (!WeakManager.IsValid() || !WeakEvent.IsValid()) return;
			AHotel_Operator* Op = WeakManager->GetDepartmentOperator();
			if (!Op || !Op->aPhone) return;

			Op->AddDialogueDataState("요청", EDialogueState::DS_Manager_Allocate_Room_Guest);
			Op->SetGuestDialogueDataLast(true);
			Op->aPhone->TryCalling("0");
		}, FMath::RandRange(2, 8), false);
}

bool UHotel_Outbreak_AllocateRoomGuest::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	if (EventInfo->CollectedTriggers.IsEmpty()) return false;

	const FString* Instigator = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	if (!Instigator || *Instigator != EventInfo->EventGuest->GuestName) return false;

	const FString AllocatedRoom = EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Target);

	if (Trigger.Type == EHotelTriggerType::GuestCheckIn)
	{
		const FString* CheckedInRoom = Trigger.Payload.Find(EHotelTriggerKey::RoomNumber);
		if (CheckedInRoom && *CheckedInRoom == AllocatedRoom)
		{
			return true;
		}
		Manager->MinusHRScore(20, TEXT("지시 불이행"));
		return true;
	}
	else if (Trigger.Type == EHotelTriggerType::GuestOutHotel)
	{
		Manager->MinusHRScore(20, TEXT("지시 불이행"));
		return true;
	}

	return false;
}

