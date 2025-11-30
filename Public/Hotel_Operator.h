// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Guest.h"
#include "Hotel_Phone.h"
#include "Hotel_Operator.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Operator : public AHotel_Guest
{
    GENERATED_BODY()

    AAI_Hotel_Guest_Default* AIController;
public:
    virtual void BeginPlay() override;
    virtual void CallingFailAction() override;

    virtual UDialogueDataAsset* GetDailogueData(EDialogueState DialogueState, bool isCall) override;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Phone)
    AHotel_Phone* aPhone;
};
