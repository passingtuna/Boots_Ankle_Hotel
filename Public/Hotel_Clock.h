// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_Clock.generated.h"

/**
 * 
 */
class UHotel_Manager;
class UTextBlock;
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Clock : public AHotel_Object
{
	GENERATED_BODY()
public:
	AHotel_Clock();

    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    int nGameTime;
    int nNextEventTime;
    int nEnviromentalLevle;
    bool bIsStopTime;

    int LastDisplayedGameTime = -1;
    FTimerHandle ClockTimer;

    UHotel_Manager* Hotel_Manager;

    UFUNCTION()
    void AddMinute();

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
    void StopGameClock();
    void ChangeEviromentLevel(int Level) { nEnviromentalLevle = Level; };
    UTextBlock* Text;
    virtual void SetHighLightInteractive(bool OnOff) override;
};
