#include "Hotel_Event_StareUnderLight.h"

#include "Hotel_Guest.h"
#include "Hotel_Light.h"
#include "Hotel_Manager.h"
#include "Hotel_Place.h"
#include "Hotel_Switch.h"
#include "Hotel_Walker.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UHotel_Event_StareUnderLight::UHotel_Event_StareUnderLight()
{
	EventID = EHotelEventId::StareUnderLight;
	ExecuteTiming = FET_WalkerEnterCounter;
	bEventOnlyOnce = false;
	bExecutingOnlyEvent = false;
}

void UHotel_Event_StareUnderLight::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return;
	EventInfo->isAreadyExcute = true;

	Manager->ActivateGuest(EventInfo->EventGuest, false);
	EventInfo->EventGuest->IsWierdStareUnderLight = true;

	if (AHotel_Light* StreetLight = Manager->GetStreetLight())
	{
		EventInfo->EventGuest->TeleportTo(FVector(-373, -100, 17), StreetLight->GetActorRotation());
	}

	EventInfo->EventGuest->SetAutoDoor(Manager->GetAutoDoor());
	EventInfo->EventGuest->SetLookingPlayer(true);
}

void UHotel_Event_StareUnderLight::TriggerNoLook(UHotel_Manager* Manager)
{
	if (!Manager) return;

	if (AHotel_Place* Lobby = Manager->GetPlaceByName("Lobby"))
	{
		if (IsValid(Lobby) && IsValid(Lobby->PlaceSwtich))
		{
			Lobby->PlaceSwtich->EventLightAction("FlickingOnce");
		}
	}

	if (AHotel_Light* StreetLight = Manager->GetStreetLight())
	{
		StreetLight->FlickeringOnce();
	}

	Manager->OnEventTriggerAction(
		FHotelTrigger::Make(EHotelTriggerType::LookStateChange,
			{
				{ EHotelTriggerKey::Instigator, TEXT("Walker") },
				{ EHotelTriggerKey::Target, EventInfo->EventGuest->GuestName },
				{ EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::NoLook) },
			}));
}

bool UHotel_Event_StareUnderLight::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, const FHotelTrigger& Trigger)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;
	UWorld* World = Manager->GetWorld();
	if (!World) return false;
	const FString* ReportTarget = Trigger.Payload.Find(EHotelTriggerKey::Target);
	const FString* SwitchPlace = Trigger.Payload.Find(EHotelTriggerKey::Place);
	const FString* SwitchState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
	const FString* LookInstigator = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
	const bool bSecurityReportLobby = (Trigger.Type == EHotelTriggerType::SecurityReport && ReportTarget && *ReportTarget == "Lobby");
	const bool bLookAtGuest = (Trigger.Type == EHotelTriggerType::LookStateChange
		&& LookInstigator && ReportTarget && SwitchState
		&& *LookInstigator == TEXT("Walker")
		&& *ReportTarget == EventInfo->EventGuest->GuestName
		&& HotelTriggerStateEquals(SwitchState, EHotelObjectState::Look));
	const bool bNoLookAtGuest = (Trigger.Type == EHotelTriggerType::LookStateChange
		&& LookInstigator && ReportTarget && SwitchState
		&& *LookInstigator == TEXT("Walker")
		&& *ReportTarget == EventInfo->EventGuest->GuestName
		&& HotelTriggerStateEquals(SwitchState, EHotelObjectState::NoLook));
	const bool bSwitchOff = (Trigger.Type == EHotelTriggerType::SwitchStateChange
		&& SwitchPlace && SwitchState
		&& *SwitchPlace == "1F"
		&& HotelTriggerStateEquals(SwitchState, EHotelObjectState::Off));
	const bool bLastSecurityReportLobby = !EventInfo->CollectedTriggers.IsEmpty()
		&& EventInfo->CollectedTriggers.Last().Type == EHotelTriggerType::SecurityReport
		&& EventInfo->CollectedTriggers.Last().Payload.FindRef(EHotelTriggerKey::Target) == "Lobby";

	if (bSecurityReportLobby && EventInfo->CollectedTriggers.Num() > 5)
	{
		EventInfo->CollectedTriggers.Add(Trigger);
	}
	else if (bLookAtGuest)
	{
		if (bLastSecurityReportLobby) return false;
		if (EventInfo->CollectedTriggers.IsEmpty() || EventInfo->CollectedTriggers.Last().Type != Trigger.Type)
		{
			EventInfo->CollectedTriggers.Add(Trigger);
		}

		TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
		World->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager]()
			{
				if (WeakManager.IsValid())
				{
					TriggerNoLook(WeakManager.Get());
				}
			}, FMath::RandRange(10, 12), false);
	}
	else if (bNoLookAtGuest || bSwitchOff)
	{
		if (bLastSecurityReportLobby)
		{
			EventInfo->EventGuest->TeleportTo(FVector(-327, 839, 94), FRotator(0, 0, 0), false, false);
			EventInfo->EventGuest->DeactivateGuest();
			Manager->UpdateDefualtLevelMenual(static_cast<int>(EventID));
			return true;
		}

		World->GetTimerManager().ClearTimer(EventInfo->EventTimer);
		EventInfo->EventGuest->ClearWalkerLookingAtGuestFlag();

		if ((!EventInfo->CollectedTriggers.IsEmpty() && EventInfo->CollectedTriggers.Last().Type != Trigger.Type))
		{
			EventInfo->CollectedTriggers.Add(Trigger);
		}

		AHotel_Light* StreetLight = Manager->GetStreetLight();

		switch (EventInfo->CollectedTriggers.Num())
		{
		case 2:
			if (StreetLight) StreetLight->SetSoftFlickering();
			break;
		case 4:
			EventInfo->EventGuest->TeleportTo(FVector(-770, 1111, 96), FRotator(0.0f, 0.0f, 0.0f));
			if (StreetLight) StreetLight->SetFlickeringOnce();
			break;
		case 6:
			EventInfo->EventGuest->TeleportTo(FVector(-1080, 1680, 96), FRotator(0.0f, 0.0f, 0.0f));
			if (StreetLight) StreetLight->SetFlickeringOnce();
			break;
		case 8:
			if (Manager->GetWalkerLocation() == "Counter")
			{
				EventInfo->EventGuest->TeleportTo(FVector(-1139, 2063, 96), FRotator(0.0f, 0.0f, 0.0f));
			}
			else if (AHotel_Walker* Walker = Manager->GetHotelWalker())
			{
				EventInfo->EventGuest->TeleportTo((Walker->FollowCamera->GetComponentLocation() + (Walker->FollowCamera->GetForwardVector() * 50)), FRotator(0.0f, 0.0f, 0.0f), false, false);
			}
			break;
		case 10:
		{
			AHotel_Walker* Walker = Manager->GetHotelWalker();
			UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Manager->GetWorld());
			if (Walker && NavSys && EventInfo->EventGuest->Controller && EventInfo->EventGuest->Controller->GetPawn())
			{
				const FVector StartLocation = EventInfo->EventGuest->Controller->GetPawn()->GetActorLocation();
				const FVector EndLocation = Walker->GetActorLocation();
				UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(Manager->GetWorld(), StartLocation, EndLocation);

				if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() >= 3)
				{
					FVector PointBeforeGoal = NavPath->PathPoints[NavPath->PathPoints.Num() - 2];
					PointBeforeGoal.Z += 90.0f;
					EventInfo->EventGuest->TeleportTo(PointBeforeGoal, FRotator(0.0f, 0.0f, 0.0f), false, true);
				}
				else
				{
					EventInfo->EventGuest->TeleportTo((Walker->FollowCamera->GetComponentLocation() + (Walker->FollowCamera->GetForwardVector() * -50)), FRotator(0.0f, 0.0f, 0.0f), false, true);
				}
			}
		}
		break;
		case 12:
			EventInfo->EventGuest->CatchingPlayer();
			Manager->UpdateDefualtLevelMenual(static_cast<int>(EventID));
			if (AHotel_Place* Lobby = Manager->GetPlaceByName("Lobby"))
			{
				if (IsValid(Lobby) && IsValid(Lobby->PlaceSwtich))
				{
					Lobby->PlaceSwtich->EventLightAction("FlickingHard");
				}
			}
			break;
		}
	}

	return false;
}

