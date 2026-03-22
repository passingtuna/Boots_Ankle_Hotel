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
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, true);

	if (IsValid(EventInfo->EventGuest))
	{
		EventInfo->EventGuest->IsCheckTrigger = true;
		EventInfo->EventGuest->IsWierdHanging = (FMath::RandRange(0, 1) == 0) ? false : true;
	}
}

bool UHotel_Event_GuestHanging::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	UWorld* World = Manager->GetWorld();
	if (!World) return false;

	AAI_Hotel_Guest_Default* AI = EventInfo->EventGuest->GetAIController();
	if (!AI || !AI->AssignedGuestRoom) return false;

	const FString* GuestNamePayload = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	if (Trigger.Type == EHotelTriggerType::GuestOutHotel
		&& GuestNamePayload
		&& *GuestNamePayload == EventInfo->EventGuest->GuestName)
	{
		return true;
	}

	if (EventInfo->CollectedTriggers.Num() == 0)
	{
		const FString* RoomNumberPayload = Trigger.Payload.Find(EHotelTriggerKey::RoomNumber);
		if (Trigger.Type == EHotelTriggerType::GuestCheckIn
			&& GuestNamePayload && RoomNumberPayload
			&& *GuestNamePayload == EventInfo->EventGuest->GuestName
			&& *RoomNumberPayload == AI->AssignedGuestRoom->RoomNumber.ToString())
		{
			EventInfo->CollectedTriggers.Add(Trigger);
		}
		return false;
	}

	const FString RoomNumber = AI->AssignedGuestRoom->RoomNumber.ToString();
	const FString* ReportTarget = Trigger.Payload.Find(EHotelTriggerKey::Target);
	const bool bSecurityReportForRoom = (Trigger.Type == EHotelTriggerType::SecurityReport && ReportTarget && *ReportTarget == RoomNumber);

	if (EventInfo->EventGuest->IsWierdHanging)
	{
		if (bSecurityReportForRoom)
		{
			EventInfo->EventGuest->SetActorHiddenInGame(true);
			EventInfo->EventGuest->TeleportTo(FVector(-327, 839, 94), FRotator(0, 0, 0));
			EventInfo->EventGuest->CheckOutGuest();
			EventInfo->CollectedTriggers.Add(Trigger);
			Manager->UpdateDefualtLevelMenual(static_cast<int>(EventID));
			return true;
		}
	}
	else
	{
		if (Trigger.Type == EHotelTriggerType::GuestHangingNeck
			&& GuestNamePayload
			&& *GuestNamePayload == EventInfo->EventGuest->GuestName)
		{
			Manager->UpdateDefualtLevelMenual(static_cast<int>(EventID));
			return true;
		}
	}

	if (Trigger.Type == EHotelTriggerType::PlaceStateChange
		&& GuestNamePayload
		&& *GuestNamePayload == EventInfo->EventGuest->GuestName)
	{
		const FString* PlaceState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
		if (!HotelTriggerStateEquals(PlaceState, EHotelObjectState::In)) return false;
		TWeakObjectPtr<AHotel_Guest> WeakGuest(EventInfo->EventGuest);
		World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakGuest]()
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

