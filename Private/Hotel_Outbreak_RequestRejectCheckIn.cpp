// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Outbreak_RequestRejectCheckIn.h"

#include "Hotel_Manager.h"
#include "Hotel_Operator.h"
#include "Hotel_Phone.h"

UHotel_Outbreak_RequestRejectCheckIn::UHotel_Outbreak_RequestRejectCheckIn()
{
	OutbreakId = EHotelOutbreakEventId::RequestRejectCheckIn;
}

void UHotel_Outbreak_RequestRejectCheckIn::Execute(UHotel_Manager* Manager, UEventInfo* EventInfo)
{
	if (!Manager || !EventInfo) return;

	EventInfo->isAreadyExcute = true;

	TWeakObjectPtr<UHotel_Manager> WeakManager(Manager);
	TWeakObjectPtr<UEventInfo> WeakEvent(EventInfo);
	Manager->GetWorld()->GetTimerManager().SetTimer(EventInfo->EventTimer, [WeakManager, WeakEvent]()
		{
			if (!WeakManager.IsValid() || !WeakEvent.IsValid()) return;
			AHotel_Operator* Op = WeakManager->GetDepartmentOperator();
			if (!Op || !Op->aPhone) return;

			Op->AddDialogueDataState("요청", EDialogueState::DS_Manager_Request_Reject_CheckIn);
			Op->SetGuestDialogueDataLast(true);
			Op->aPhone->TryCalling("0");
		}, FMath::RandRange(2, 8), false);
}

bool UHotel_Outbreak_RequestRejectCheckIn::CheckClear(UHotel_Manager* Manager, UEventInfo* EventInfo, FName TriggerName)
{
	if (!Manager || !EventInfo || !IsValid(EventInfo->EventGuest)) return false;

	TArray<FString> Parts;
	TriggerName.ToString().ParseIntoArray(Parts, TEXT("_"), true);
	if (Parts.Num() < 2) return false;

	if (Parts[0] != EventInfo->EventGuest->GuestName) return false;

	if (Parts[1] == "CheckIn")
	{
		Manager->MinusHRScore(20, TEXT("지시 불이행"));

		if (AHotel_Operator* Op = Manager->GetDepartmentOperator())
		{
			Op->AddDialogueDataState("요청", EDialogueState::DS_Manager_Request_Reject_CheckIn);
			Op->SetGuestDialogueDataLast(true);
		}
		return true;
	}
	else if (Parts[1] == "OutHotel")
	{
		return true;
	}

	return false;
}

