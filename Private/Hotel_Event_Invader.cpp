// Fill out your copyright notice in the Description page of Project Settings.

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

	UE_LOG(LogTemp, Warning, TEXT("인베이더 출몰"));
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, false);
	EventInfo->EventGuest->IsReadyToNeckShaking = true;

	if (AHotel_Door* StaffDoor = Manager->GetStaffDoor())
	{
		StaffDoor->GetLockState() ? Manager->OnEventTriggerAction("StaffDoor_Lock") : Manager->OnEventTriggerAction("StaffDoor_Unlock");
	}
}

void UHotel_Event_Invader::TriggerInvade(UHotel_Manager* Manager)
{
	if (Manager)
	{
		Manager->OnEventTriggerAction("InvadeInvader");
	}
}

bool UHotel_Event_Invader::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;

	const FName WalkerLocation = Manager->GetWalkerLocation();

	if (!EventInfo->CollectedTriggers.IsEmpty() && EventInfo->CollectedTriggers.Last() == "InvadeInvader")
	{
		if (TriggerName == "SecurityReport_0")
		{
			EventInfo->EventGuest->TeleportTo(FVector(0, 1343, 94), FRotator(0, 0, 0), false, false);
			EventInfo->EventGuest->DeactivateGuest();
			EventInfo->CollectedTriggers.Add(TriggerName);
			return true;
		}
		else if (WalkerLocation == "StaffRoom" || TriggerName == "StaffDoor_Open")
		{
			EventInfo->EventGuest->CatchingPlayer();
		}
	}
	else if (TriggerName == "StaffDoor_Lock")
	{
		if (!EventInfo->CollectedTriggers.IsEmpty() && EventInfo->CollectedTriggers.Last() == "InvadeInvader") return false;
		Manager->GetWorld()->GetTimerManager().ClearTimer(EventInfo->EventTimer);
		EventInfo->CollectedTriggers.Add(TriggerName);
	}
	else if (TriggerName == "StaffDoor_Unlock")
	{
		if (!EventInfo->CollectedTriggers.IsEmpty() && (EventInfo->CollectedTriggers.Last() == "InvadeInvader" || EventInfo->CollectedTriggers.Last() == "StaffDoor_Unlock")) return false;

		TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
		Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager]()
			{
				if (WeakManager.IsValid())
				{
					TriggerInvade(WeakManager.Get());
				}
			}, FMath::RandRange(10, 15), false);

		EventInfo->CollectedTriggers.Add(TriggerName);
	}
	else if (TriggerName == "InvadeInvader")
	{
		Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
		EventInfo->CollectedTriggers.Add(TriggerName);

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

