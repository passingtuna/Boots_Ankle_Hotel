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
}

void UHotel_Outbreak_AllocateRoomGuest::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

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

	EventInfo->CollectedTriggers.Add(FName("AllocateRoomNum_" + EmptyRooms[FMath::RandRange(0, EmptyRooms.Num() - 1)]));

	TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
	TWeakObjectPtr<UEventInfo> WeakEvent(EventInfo);
	Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, WeakEvent]()
		{
			if (!WeakManager.IsValid() || !WeakEvent.IsValid()) return;
			AHotel_Operator* Op = WeakManager->GetDepartmentOperator();
			if (!Op || !Op->aPhone) return;

			Op->AddDialogueDataState("요청", EDialogueState::DS_Manager_Allocate_Room_Guest);
			Op->SetGuestDialogueDataLast(true);
			Op->aPhone->TryCalling("0");
		}, FMath::RandRange(2, 8), false);
}

bool UHotel_Outbreak_AllocateRoomGuest::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	if (EventInfo->CollectedTriggers.IsEmpty()) return false;

	TArray<FString> TriggerParts;
	TriggerName.ToString().ParseIntoArray(TriggerParts, TEXT("_"), true);
	if (TriggerParts.Num() < 2) return false;

	TArray<FString> AllocParts;
	EventInfo->CollectedTriggers.Last().ToString().ParseIntoArray(AllocParts, TEXT("_"), true);
	if (AllocParts.Num() < 2) return false;

	if (TriggerParts[0] != EventInfo->EventGuest->GuestName) return false;

	if (TriggerParts[1] == "CheckIn")
	{
		if (TriggerParts.Num() >= 3 && TriggerParts[2] == AllocParts[1])
		{
			return true;
		}
		Manager->MinusHRScore(20, TEXT("지시 불이행"));
		return true;
	}
	else if (TriggerParts[1] == "OutHotel")
	{
		Manager->MinusHRScore(20, TEXT("지시 불이행"));
		return true;
	}

	return false;
}

