// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Outbreak_MakeDirtyRoom.h"

#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"
#include "Hotel_Door.h"

UHotel_Outbreak_MakeDirtyRoom::UHotel_Outbreak_MakeDirtyRoom()
{
	OutbreakId = EHotelOutbreakEventId::MakeDirtyRoom;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = true;
}

void UHotel_Outbreak_MakeDirtyRoom::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager) return;

	const TMap<FName, FRoomInfo>& Rooms = Manager->GetRegisteredRooms();
	for (const auto& Kvp : Rooms)
	{
		const FRoomInfo& Info = Kvp.Value;
		if (!IsValid(Info.RoomGuest) && Info.Room)
		{
			if (Info.Room->RoomNumber != "205" && Info.Room->RoomNumber != "305")
			{
				Info.Room->MakeRoomDirty(3);
				if (Info.Room->aDoor)
				{
					Info.Room->aDoor->SetLockDoor(false);
					Info.Room->aDoor->SetOpenDoor(true);
				}
				return;
			}
		}
	}
}

bool UHotel_Outbreak_MakeDirtyRoom::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	(void)Manager;
	(void)EventInfo;
	(void)Trigger;
	return false;
}

