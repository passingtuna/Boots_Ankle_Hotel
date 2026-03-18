// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Event_Open205.h"

#include "Hotel_Guest.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"
#include "Hotel_StaticMesh.h"
#include "Hotel_Door.h"

UHotel_Event_Open205::UHotel_Event_Open205()
{
	EventID = EHotelEventId::Open205;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = true;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_Open205::Fail205(UHotel_Manager* Manager)
{
	if (!Manager) return;

	if (UHotel_StaticMesh* Mesh = Manager->GetHotelMeshByName("205Wall"))
	{
		Mesh->SetMeshState(true);
	}
	Manager->MinusHRScore(20, TEXT("205호 격리 절차 미이행"));
}

void UHotel_Event_Open205::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

	UE_LOG(LogTemp, Warning, TEXT("205호 등장"));
	EventInfo->isAreadyExcute = true;

	if (UHotel_StaticMesh* Mesh = Manager->GetHotelMeshByName("205Wall"))
	{
		Mesh->SetMeshState(false);
	}

	if (FRoomInfo* RoomInfo = Manager->FindRoomInfoByName("205"))
	{
		if (RoomInfo->Room)
		{
			Manager->ActivateGuest(EventInfo->EventGuest, false);

			TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
			Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager]()
				{
					if (WeakManager.IsValid())
					{
						Fail205(WeakManager.Get());
					}
				}, 40.0f, false);

			RoomInfo->Room->HangingNeck(EventInfo->EventGuest);
			RoomInfo->Room->SetRoomFlickingLight(2);
			RoomInfo->Room->SetActorHiddenInGame(false);
		}
	}
}

bool UHotel_Event_Open205::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo) return false;

	if (TriggerName == "Walker_Enter_205")
	{
		if (UHotel_StaticMesh* Mesh = Manager->GetHotelMeshByName("205Wall"))
		{
			Mesh->SetMeshState(true);
		}

		if (FRoomInfo* RoomInfo = Manager->FindRoomInfoByName("205"))
		{
			if (RoomInfo->Room && RoomInfo->Room->aDoor)
			{
				RoomInfo->Room->aDoor->SetOpenDoor(false);
				RoomInfo->Room->aDoor->SetUncontrolableLock(true);
			}
		}

		Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
		EventInfo->CollectedTriggers.Add(TriggerName);
		Manager->GetWorld()->GetTimerManager().ClearTimer(EventInfo->EventTimer);
	}
	else if (TriggerName == "WalkerTryCall_205_To_205" && !EventInfo->CollectedTriggers.IsEmpty() && EventInfo->CollectedTriggers.Last() == "Walker_Enter_205")
	{
		if (UHotel_StaticMesh* Mesh = Manager->GetHotelMeshByName("205Wall"))
		{
			Mesh->SetMeshState(false);
		}
		if (FRoomInfo* RoomInfo = Manager->FindRoomInfoByName("205"))
		{
			if (RoomInfo->Room && RoomInfo->Room->aDoor)
			{
				RoomInfo->Room->aDoor->SetUncontrolableLock(false);
				RoomInfo->Room->aDoor->SetOpenDoor(true);
			}
		}
		EventInfo->CollectedTriggers.Add(TriggerName);
	}
	else if (TriggerName == "Walker_Enter_2F" && !EventInfo->CollectedTriggers.IsEmpty() && EventInfo->CollectedTriggers.Last() == "WalkerTryCall_205_To_205")
	{
		if (UHotel_StaticMesh* Mesh = Manager->GetHotelMeshByName("205Wall"))
		{
			Mesh->SetMeshState(true);
		}
		if (FRoomInfo* RoomInfo = Manager->FindRoomInfoByName("205"))
		{
			if (RoomInfo->Room && RoomInfo->Room->aDoor)
			{
				RoomInfo->Room->aDoor->SetOpenDoor(false);
				RoomInfo->Room->aDoor->SetUncontrolableLock(true);
			}
		}
		return true;
	}
	else if (TriggerName == "GameEnd")
	{
		Fail205(Manager);
	}

	return false;
}

