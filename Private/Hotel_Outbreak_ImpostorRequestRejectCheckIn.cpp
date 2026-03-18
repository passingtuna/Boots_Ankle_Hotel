// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Outbreak_ImpostorRequestRejectCheckIn.h"

#include "Hotel_Manager.h"
#include "Hotel_Operator.h"
#include "Hotel_Phone.h"

UHotel_Outbreak_ImpostorRequestRejectCheckIn::UHotel_Outbreak_ImpostorRequestRejectCheckIn()
{
	OutbreakId = EHotelOutbreakEventId::ImpostorRequestRejectCheckIn;
}

void UHotel_Outbreak_ImpostorRequestRejectCheckIn::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

	TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
	TWeakObjectPtr<UEventInfo> WeakEvent(EventInfo);
	Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, WeakEvent]()
		{
			if (!WeakManager.IsValid() || !WeakEvent.IsValid()) return;
			AHotel_Operator* Op = WeakManager->GetDepartmentOperator();
			if (!Op || !Op->aPhone) return;

			Op->AddDialogueDataState("요청", EDialogueState::DS_Impostor_Request_Reject_CheckIn);
			Op->SetGuestDialogueDataLast(true);
			Op->aPhone->TryCalling("0");
		}, FMath::RandRange(2, 8), false);
}

bool UHotel_Outbreak_ImpostorRequestRejectCheckIn::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	return false;
}

