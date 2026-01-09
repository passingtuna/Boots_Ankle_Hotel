// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_Switch.generated.h"

class AHotel_Light;
/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Switch : public AHotel_Object
{
	GENERATED_BODY()
	
    virtual void BeginPlay() override;

private:
    bool isSwitchOn;
public:
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    TArray<AHotel_Light *> arrConnectedLight;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    USceneComponent* comSwitchButton;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite)
    FName SwitchName ="";
    void SetSwitchToggle();

    void SetSwitchName(FName name);
    UFUNCTION()
    void AddConnectedLight(AHotel_Light* AddLight);

    void EventLightAction(FName nActionType);
};
