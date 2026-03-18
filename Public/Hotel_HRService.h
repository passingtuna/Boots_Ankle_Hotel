// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Types.h"
#include "Hotel_HRService.generated.h"

class AHotel_Operator;

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_HRService : public UObject
{
	GENERATED_BODY()

public:
	void ResetForNewGame();
	void ResetForContinue(int CurrentDay);

	int GetScore() const { return HumanResourcesScore; }
	TArray<FHRRecord> GetRecords() const { return HRRecords; }
	bool GetWalkerFired() const { return bManagerFiredWalker; }

	void MinusScore(int MinusScore, const FString& Reason, UWorld* World, AHotel_Operator* DepartmentOperator, const FName& WalkerNowLocation);
	void PlusScore(int PlusScore, const FString& Reason);

	void CheckManagerCallingForHR(AHotel_Operator* DepartmentOperator, const FName& WalkerNowLocation);

private:
	UPROPERTY()
	int HumanResourcesScore = 100;

	UPROPERTY()
	TArray<FHRRecord> HRRecords;

	UPROPERTY()
	bool bManagerWarningWalker = false;

	UPROPERTY()
	bool bManagerFiredWalker = false;
};

