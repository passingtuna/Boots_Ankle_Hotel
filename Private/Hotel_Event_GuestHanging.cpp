// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Event_GuestHanging.h"

#include "AI_Hotel_Guest_Default.h"
#include "Hotel_Guest.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"

UHotel_Event_GuestHanging::UHotel_Event_GuestHanging()
{
	EventID = EHotelEventId::GuestHanging;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_GuestHanging::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

	UE_LOG(LogTemp, Warning, TEXT("행잉 게스트"));
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, true);

	if (IsValid(EventInfo->EventGuest))
	{
		EventInfo->EventGuest->IsCheckTrigger = true;
		EventInfo->EventGuest->IsWierdHanging = (FMath::RandRange(0, 1) == 0) ? false : true;
	}
}

bool UHotel_Event_GuestHanging::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;

	AAI_Hotel_Guest_Default* AI = EventInfo->EventGuest->GetAIController();
	if (!AI || !AI->AssignedGuestRoom) return false;

	if (FName(EventInfo->EventGuest->GuestName + "_OutHotel") == TriggerName)
	{
		return true;
	}

	if (EventInfo->CollectedTriggers.Num() == 0)
	{
		TArray<FString> Parts;
		TriggerName.ToString().ParseIntoArray(Parts, TEXT("_"), true);
		if (Parts.Num() >= 2 && Parts[0] == EventInfo->EventGuest->GuestName && Parts[1] == "CheckIn")
		{
			EventInfo->CollectedTriggers.Add(TriggerName);
		}
		return false;
	}

	const FString RoomNumber = AI->AssignedGuestRoom->RoomNumber.ToString();

	if (EventInfo->EventGuest->IsWierdHanging)
	{
		if (FName("SecurityReport_" + RoomNumber) == TriggerName)
		{
			EventInfo->EventGuest->SetActorHiddenInGame(true);
			EventInfo->EventGuest->TeleportTo(FVector(-327, 839, 94), FRotator(0, 0, 0));
			EventInfo->EventGuest->CheckOutGuest();
			EventInfo->CollectedTriggers.Add(TriggerName);
			Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
			return true;
		}
	}
	else
	{
		if (FName(EventInfo->EventGuest->GuestName + "_HangingNeck") == TriggerName)
		{
			Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
			return true;
		}
	}

	const FName CheckTriggerName = FName(EventInfo->EventGuest->GuestName + "_In_" + RoomNumber);
	if (CheckTriggerName == TriggerName)
	{
		TWeakObjectPtr<AHotel_Guest> WeakGuest(EventInfo->EventGuest);
		Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakGuest]()
			{
				if (!WeakGuest.IsValid()) return;
				AAI_Hotel_Guest_Default* LocalAI = WeakGuest->GetAIController();
				if (!LocalAI || !LocalAI->AssignedGuestRoom) return;

				LocalAI->AssignedGuestRoom->SetRoomFlickingLight(1);
				LocalAI->AssignedGuestRoom->HangingNeck(WeakGuest.Get());
				LocalAI->StopAITimer();
				LocalAI->StopPatienceTimer();
				UE_LOG(LogTemp, Warning, TEXT("행잉"));
			}, FMath::RandRange(10, 20), false);
	}

	return false;
}

