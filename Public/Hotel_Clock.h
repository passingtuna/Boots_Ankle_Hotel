// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Components/TextBlock.h"
#include "Hotel_Clock.generated.h"

/**
 * 
 */
class UHotel_Manager;
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Clock : public AHotel_Object
{
	GENERATED_BODY()
	
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    int nGameTime;
    int nNextEventTime;
    int nEnviromentalLevle;
    bool bIsStopTime;

    FTimerHandle ClockTimer;

    UHotel_Manager* Hotel_Manager;

    UFUNCTION()
    void AddMinute();

public:
    UPROPERTY(EditAnyWhere,BlueprintReadWrite, Category = "Needle")
    USceneComponent* HourNeedleComp;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Needle")
    USceneComponent* MinuteNeedleComp;

    void SetTime(int Hour, int Minute)
    {
        nGameTime = Hour * 60 + Minute;
    };

    void SetStopTime(bool on) { bIsStopTime = on; };
    void StartGameClock();
    UTextBlock* Text;
    virtual void SetHighLightInteractive(bool OnOff) override;
};
