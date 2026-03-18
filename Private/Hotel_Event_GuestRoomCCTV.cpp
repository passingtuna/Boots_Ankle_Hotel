// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Event_GuestRoomCCTV.h"

#include "Hotel_CCTV.h"
#include "Hotel_CCTV_Camera.h"
#include "Hotel_Guest.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"

UHotel_Event_GuestRoomCCTV::UHotel_Event_GuestRoomCCTV()
{
	EventID = EHotelEventId::GuestRoomCCTV;
	ExecuteTiming = FET_Init;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_GuestRoomCCTV::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

	EventInfo->isAreadyExcute = true;
	Manager->ActivateGuest(EventInfo->EventGuest, true);
	if (IsValid(EventInfo->EventGuest))
	{
		EventInfo->EventGuest->IsCheckTrigger = true;
	}
}

bool UHotel_Event_GuestRoomCCTV::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;

	if (!EventInfo->EventGuest->GetAIController() || !EventInfo->EventGuest->GetAIController()->AssignedGuestRoom) return false;
	AHotel_Guest_Room* GuestRoom = EventInfo->EventGuest->GetAIController()->AssignedGuestRoom;
	const FString RoomNumber = GuestRoom->RoomNumber.ToString();

	AHotel_CCTV* CCTV = Manager->GetCCTV();
	if (!CCTV) return false;

	if (FName(EventInfo->EventGuest->GuestName + "_In_" + RoomNumber) == TriggerName)
	{
		TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
		TWeakObjectPtr<AHotel_Guest> WeakGuest(EventInfo->EventGuest);
		Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, WeakGuest]()
			{
				if (!WeakManager.IsValid() || !WeakGuest.IsValid()) return;
				AHotel_Guest_Room* R = WeakGuest->GetAIController() ? WeakGuest->GetAIController()->AssignedGuestRoom : nullptr;
				if (!R) return;
				if (AHotel_CCTV* LocalCCTV = WeakManager->GetCCTV())
				{
					LocalCCTV->ActiveRoomCameraToCCTV(R->aCCTV_Camera);
				}
			}, 1.0f, false);

		GuestRoom->SetIsolationState(true);
	}

	TArray<FString> Parts;
	TriggerName.ToString().ParseIntoArray(Parts, TEXT("_"), true);

	if (EventInfo->CollectedTriggers.Num() < 7)
	{
		if (TriggerName == "Walker_Out_Counter")
		{
			if (CCTV->GetRoomCameraIndex(GuestRoom->aCCTV_Camera->CameraName) == CCTV->GetNowCameraNum())
			{
				Manager->GetWorld()->GetTimerManager().ClearTimer(EventInfo->EventTimer);
				for (int i = EventInfo->CollectedTriggers.Num(); i < 5; i++)
				{
					EventInfo->CollectedTriggers.Add(FName("ViewCCTV_" + RoomNumber));
					EventInfo->EventGuest->SetLookingCameraStatue(EventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
				}
			}
		}
		else if (TriggerName == "Walker_Enter_Counter")
		{
			if (CCTV->GetRoomCameraIndex(GuestRoom->aCCTV_Camera->CameraName) == CCTV->GetNowCameraNum())
			{
				Manager->GetWorld()->GetTimerManager().ClearTimer(EventInfo->EventTimer);
				EventInfo->CollectedTriggers.Add(TriggerName);
				EventInfo->EventGuest->SetLookingCameraStatue(EventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
				Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
			}
		}
		else if (Parts.Num() >= 2 && Parts[0] == "ViewCCTV")
		{
			if (Parts[1] == RoomNumber)
			{
				EventInfo->CollectedTriggers.Add(TriggerName);
				EventInfo->EventGuest->SetLookingCameraStatue(EventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
				const FName Copy = TriggerName;
				TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
				Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, Copy]()
					{
						if (WeakManager.IsValid())
						{
							WeakManager->OnEventTriggerAction(Copy);
						}
					}, 2.0f, false);
			}
			else
			{
				Manager->GetWorld()->GetTimerManager().ClearTimer(EventInfo->EventTimer);
			}
		}
	}

	return false;
}

