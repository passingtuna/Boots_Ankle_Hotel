#include "Hotel_Event_Invader.h"

#include "Hotel_Door.h"
#include "Hotel_Guest.h"
#include "Hotel_Manager.h"

UHotel_Event_Invader::UHotel_Event_Invader()
{
	EventID = EHotelEventId::Invader;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_Invader::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return;
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, false);
	EventInfo->EventGuest->IsReadyToNeckShaking = true;

	if (AHotel_Door* StaffDoor = Manager->GetStaffDoor())
	{
		if (StaffDoor->GetLockState())
		{
			Manager->OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::DoorStateChange,
				{
					{ EHotelTriggerKey::Place, TEXT("Staff") },
					{ EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::Lock) },
				}));
		}
		else
		{
			Manager->OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::DoorStateChange,
				{
					{ EHotelTriggerKey::Place, TEXT("Staff") },
					{ EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::UnLock) },
				}));
		}
	}
}

void UHotel_Event_Invader::TriggerInvade(UHotel_Manager* Manager)
{
	if (Manager)
	{
		Manager->OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::InvadeInvader));
	}
}

bool UHotel_Event_Invader::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	UWorld* World = Manager->GetWorld();
	if (!World) return false;

	const FName WalkerLocation = Manager->GetWalkerLocation();
	const FString* ReportTarget = Trigger.Payload.Find(EHotelTriggerKey::Target);
	const FString* DoorPlace = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* DoorState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	const bool bSecurityReportLobby = (Trigger.Type == EHotelTriggerType::SecurityReport && ReportTarget && *ReportTarget == "Lobby");
	const bool bStaffDoorOpen = (Trigger.Type == EHotelTriggerType::DoorStateChange
		&& DoorPlace && DoorState
		&& *DoorPlace == TEXT("Staff")
		&& HotelTriggerStateEquals(DoorState, EHotelObjectState::Open));
	const bool bStaffDoorLock = (Trigger.Type == EHotelTriggerType::DoorStateChange
		&& DoorPlace && DoorState
		&& *DoorPlace == TEXT("Staff")
		&& HotelTriggerStateEquals(DoorState, EHotelObjectState::Lock));
	const bool bStaffDoorUnlock = (Trigger.Type == EHotelTriggerType::DoorStateChange
		&& DoorPlace && DoorState
		&& *DoorPlace == TEXT("Staff")
		&& HotelTriggerStateEquals(DoorState, EHotelObjectState::UnLock));
	const bool bInvadeInvader = (Trigger.Type == EHotelTriggerType::InvadeInvader);

	const bool bLastInvade = !EventInfo->CollectedTriggers.IsEmpty() && EventInfo->CollectedTriggers.Last().Type == EHotelTriggerType::InvadeInvader;
	const bool bLastStaffDoorUnlock = !EventInfo->CollectedTriggers.IsEmpty()
		&& EventInfo->CollectedTriggers.Last().Type == EHotelTriggerType::DoorStateChange
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Place) == TEXT("Staff")
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::ObjectState) == HotelTriggerStateToString(EHotelObjectState::UnLock);
	if (bLastInvade)
	{
		if (bSecurityReportLobby)
		{
			EventInfo->EventGuest->TeleportTo(FVector(0, 1343, 94), FRotator(0, 0, 0), false, false);
			EventInfo->EventGuest->DeactivateGuest();
			EventInfo->CollectedTriggers.Add(Trigger);
			return true;
		}
		else if (WalkerLocation == "StaffRoom" || bStaffDoorOpen)
		{
			EventInfo->EventGuest->CatchingPlayer();
		}
	}
	else if (bStaffDoorLock)
	{
		if (bLastInvade) return false;
		World->GetTimerManager().ClearTimer(EventInfo->EventTimer);
		EventInfo->CollectedTriggers.Add(Trigger);
	}
	else if (bStaffDoorUnlock)
	{
		if (bLastInvade || bLastStaffDoorUnlock) return false;

		TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
		World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager]()
			{
				if (WeakManager.IsValid())
				{
					TriggerInvade(WeakManager.Get());
				}
			}, FMath::RandRange(10, 15), false);

		EventInfo->CollectedTriggers.Add(Trigger);
	}
	else if (bInvadeInvader)
	{
		Manager->UpdateDefualtLevelMenual(static_cast<int>(EventID));
		EventInfo->CollectedTriggers.Add(Trigger);

		if (AHotel_Door* StaffDoor = Manager->GetStaffDoor())
		{
			StaffDoor->SetOpenDoor(false);
		}

		if (WalkerLocation == "Counter")
		{
			EventInfo->EventGuest->ReadyToRotate();
			EventInfo->EventGuest->TeleportTo(FVector(-1222, 2579, 96), FRotator(0.0f, 180.0f, 0.0f), false, true);
		}
		else
		{
			EventInfo->EventGuest->TeleportTo(FVector(-744, 2594.0, 96), FRotator(0.0f, 0.0f, 0.0f), false, true);
		}
	}

	return false;
}

