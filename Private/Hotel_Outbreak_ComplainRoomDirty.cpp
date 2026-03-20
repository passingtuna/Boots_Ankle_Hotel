// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Outbreak_ComplainRoomDirty.h"

#include "AI_Hotel_Guest_Default.h"
#include "Hotel_Guest.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"

UHotel_Outbreak_ComplainRoomDirty::UHotel_Outbreak_ComplainRoomDirty()
{
	OutbreakId = EHotelOutbreakEventId::ComplainRoomDirty;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Outbreak_ComplainRoomDirty::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return;
	UWorld* World = Manager->GetWorld();
	if (!World) return;

	EventInfo->isAreadyExcute = true;
	EventInfo->EventGuest->AddDialogueDataState(TEXT("방 청소 상태에 관하여"), EDialogueState::DS_Guest_ComplainRoomCondition, 0);
	EventInfo->EventGuest->SetGuestDialogueDataLast(false);
	EventInfo->EventGuest->IsCalledWalker = true;

	TWeakObjectPtr<AHotel_Guest> WeakGuest(EventInfo->EventGuest);
	World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakGuest]()
		{
			if (!WeakGuest.IsValid()) return;
			AAI_Hotel_Guest_Default* AI = WeakGuest->GetAIController();
			if (AI && AI->AssignedGuestRoom && AI->AssignedGuestRoom->aPhone)
			{
				AI->AssignedGuestRoom->aPhone->TryCalling("0");
			}
		}, 2.0f, false);
}

bool UHotel_Outbreak_ComplainRoomDirty::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!EventInfo || !IsValid(EventInfo->EventGuest) || !EventInfo->EventGuest->GetAIController() || !EventInfo->EventGuest->GetAIController()->AssignedGuestRoom)
	{
		return false;
	}

	const FString RoomPlace = EventInfo->EventGuest->GetAIController()->AssignedGuestRoom->RoomNumber.ToString();
	const FString* CleanPlace = Trigger.Payload.Find(EHotelTriggerKey::Place);
	if (Trigger.Type == EHotelTriggerType::RoomCleaned
		&& CleanPlace && *CleanPlace == RoomPlace)
	{
		if (!EventInfo->EventGuest->GetAIController()->AssignedGuestRoom->CheckRoomDirty())
		{
			return true;
		}
	}
	return false;
}

