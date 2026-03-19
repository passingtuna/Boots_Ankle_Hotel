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

bool UHotel_Event_GuestRoomCCTV::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	UWorld* World = Manager->GetWorld();
	if (!World) return false;

	if (!EventInfo->EventGuest->GetAIController() || !EventInfo->EventGuest->GetAIController()->AssignedGuestRoom) return false;
	AHotel_Guest_Room* GuestRoom = EventInfo->EventGuest->GetAIController()->AssignedGuestRoom;
	const FString RoomPlace = GuestRoom->RoomNumber.ToString();

	AHotel_CCTV* CCTV = Manager->GetCCTV();
	if (!CCTV) return false;

	const FString* GuestNamePayload = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	const FString* PlacePayload = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* PlaceState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	if (Trigger.Type == EHotelTriggerType::PlaceStateChange
		&& GuestNamePayload && PlacePayload
		&& HotelTriggerStateEquals(PlaceState, EHotelObjectState::In)
		&& *GuestNamePayload == EventInfo->EventGuest->GuestName
		&& *PlacePayload == RoomPlace)
	{
		TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
		TWeakObjectPtr<AHotel_Guest> WeakGuest(EventInfo->EventGuest);
		World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, WeakGuest]()
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

	if (EventInfo->CollectedTriggers.Num() < 7)
	{
		const FString* Place = Trigger.Payload.Find(EHotelTriggerKey::Place);
		const FString* Instigator = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
		const FString* State = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
		const bool bWalkerOutCounter = (Trigger.Type == EHotelTriggerType::PlaceStateChange
			&& Instigator && State && Place
			&& *Instigator == TEXT("Walker")
			&& HotelTriggerStateEquals(State, EHotelObjectState::Out)
			&& *Place == "Counter");
		const bool bWalkerEnterCounter = (Trigger.Type == EHotelTriggerType::PlaceStateChange
			&& Instigator && State && Place
			&& *Instigator == TEXT("Walker")
			&& HotelTriggerStateEquals(State, EHotelObjectState::In)
			&& *Place == "Counter");
		if (bWalkerOutCounter)
		{
			if (CCTV->GetRoomCameraIndex(GuestRoom->aCCTV_Camera->CameraName) == CCTV->GetNowCameraNum())
			{
				World->GetTimerManager().ClearTimer(EventInfo->EventTimer);
				for (int i = EventInfo->CollectedTriggers.Num(); i < 5; i++)
				{
					EventInfo->CollectedTriggers.Add(
						FHotelTrigger::Make(
							EHotelTriggerType::CCTVStateChange,
							{
								{ EHotelTriggerKey::Place, RoomPlace },
								{ EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::View) },
							}));
					EventInfo->EventGuest->SetLookingCameraStatue(EventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
				}
			}
		}
		else if (bWalkerEnterCounter)
		{
			if (CCTV->GetRoomCameraIndex(GuestRoom->aCCTV_Camera->CameraName) == CCTV->GetNowCameraNum())
			{
				World->GetTimerManager().ClearTimer(EventInfo->EventTimer);
				EventInfo->CollectedTriggers.Add(Trigger);
				EventInfo->EventGuest->SetLookingCameraStatue(EventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
				Manager->UpdateDefualtLevelMenual(EventInfo->FunctionInfo.EventID);
			}
		}
		else if (Trigger.Type == EHotelTriggerType::CCTVStateChange)
		{
			const FString* CCTVPlace = Trigger.Payload.Find(EHotelTriggerKey::Place);
			const FString* CCTVState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
			if (CCTVPlace && CCTVState
				&& *CCTVPlace == RoomPlace
				&& HotelTriggerStateEquals(CCTVState, EHotelObjectState::View))
			{
				EventInfo->CollectedTriggers.Add(Trigger);
				EventInfo->EventGuest->SetLookingCameraStatue(EventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
				const FHotelTrigger Copy = Trigger;
				TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
				World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, Copy]()
					{
						if (WeakManager.IsValid())
						{
							WeakManager->OnEventTriggerAction(Copy);
						}
					}, 2.0f, false);
			}
			else
			{
				World->GetTimerManager().ClearTimer(EventInfo->EventTimer);
			}
		}
	}

	return false;
}

