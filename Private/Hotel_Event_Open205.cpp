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
	UWorld* World = Manager->GetWorld();
	if (!World) return;
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
			World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager]()
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

bool UHotel_Event_Open205::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo) return false;
	UWorld* World = Manager->GetWorld();
	if (!World) return false;
	const FString* Place = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* Instigator = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	const FString* PlaceState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	const FString* Target = Trigger.Payload.Find(EHotelTriggerKey::Target);
	const bool bEnter205 = (Trigger.Type == EHotelTriggerType::PlaceStateChange
		&& Instigator && PlaceState && Place
		&& *Instigator == TEXT("Walker")
		&& HotelTriggerStateEquals(PlaceState, EHotelObjectState::In)
		&& *Place == "205");
	const bool bTryCall205To205 = (Trigger.Type == EHotelTriggerType::TryCall && Place && Target && *Place == "205" && *Target == "205");
	const bool bEnter2F = (Trigger.Type == EHotelTriggerType::PlaceStateChange
		&& Instigator && PlaceState && Place
		&& *Instigator == TEXT("Walker")
		&& HotelTriggerStateEquals(PlaceState, EHotelObjectState::In)
		&& *Place == "2F");
	const bool bGameEnd = (Trigger.Type == EHotelTriggerType::GameEnd);
	const bool bLastEnter205 = !EventInfo->CollectedTriggers.IsEmpty()
		&& EventInfo->CollectedTriggers.Last().Type == EHotelTriggerType::PlaceStateChange
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Instigator) == TEXT("Walker")
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::ObjectState) == HotelTriggerStateToString(EHotelObjectState::In)
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Place) == "205";
	const bool bLastTryCall205To205 = !EventInfo->CollectedTriggers.IsEmpty()
		&& EventInfo->CollectedTriggers.Last().Type == EHotelTriggerType::TryCall
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Place) == "205"
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Target) == "205";

	if (bEnter205)
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
		EventInfo->CollectedTriggers.Add(Trigger);
		World->GetTimerManager().ClearTimer(EventInfo->EventTimer);
	}
	else if (bTryCall205To205 && bLastEnter205)
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
		EventInfo->CollectedTriggers.Add(Trigger);
	}
	else if (bEnter2F && bLastTryCall205To205)
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
	else if (bGameEnd)
	{
		Fail205(Manager);
	}

	return false;
}

