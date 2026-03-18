// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_HRService.h"

#include "Hotel_Operator.h"
#include "Hotel_Phone.h"
#include "TimerManager.h"

void UHotel_HRService::ResetForNewGame()
{
	HumanResourcesScore = 100;
	HRRecords.Empty();
	bManagerWarningWalker = false;
	bManagerFiredWalker = false;
}

void UHotel_HRService::ResetForContinue(int CurrentDay)
{
	HRRecords.Empty();
	bManagerWarningWalker = false;
	bManagerFiredWalker = false;
}

void UHotel_HRService::MinusScore(int MinusScore, const FString& Reason, UWorld* World, AHotel_Operator* DepartmentOperator, const FName& WalkerNowLocation)
{
	HumanResourcesScore -= MinusScore;

	FHRRecord Record;
	Record.Minus = true;
	Record.Reason = Reason;
	Record.Score = MinusScore;
	HRRecords.Add(Record);

	if (HumanResourcesScore < 0)
	{
		HumanResourcesScore = 0;
	}

	if (WalkerNowLocation == "Counter")
	{
		CheckManagerCallingForHR(DepartmentOperator, WalkerNowLocation);
	}
}

void UHotel_HRService::PlusScore(int PlusScore, const FString& Reason)
{
	HumanResourcesScore += PlusScore;
	if (HumanResourcesScore > 100) HumanResourcesScore = 100;

	FHRRecord Record;
	Record.Minus = false;
	Record.Reason = Reason;
	Record.Score = PlusScore;
	HRRecords.Add(Record);
}

void UHotel_HRService::CheckManagerCallingForHR(AHotel_Operator* DepartmentOperator, const FName& WalkerNowLocation)
{
	if (!DepartmentOperator || !DepartmentOperator->aPhone) return;

	if (!bManagerFiredWalker && HumanResourcesScore <= 0)
	{
		DepartmentOperator->AddDialogueDataState("해고", EDialogueState::DS_Manager_Fire);
		DepartmentOperator->SetGuestDialogueDataLast(true);
		DepartmentOperator->EraseDialgueDataState(EDialogueState::DS_Manager_Warning);
		DepartmentOperator->aPhone->TryCalling("0");
		bManagerFiredWalker = true;
		bManagerWarningWalker = true;
	}
	else if (!bManagerWarningWalker && HumanResourcesScore <= 50)
	{
		DepartmentOperator->AddDialogueDataState("경고", EDialogueState::DS_Manager_Warning);
		DepartmentOperator->SetGuestDialogueDataLast(true);
		DepartmentOperator->aPhone->TryCalling("0");
		bManagerWarningWalker = true;
	}
}

