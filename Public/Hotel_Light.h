// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
#include "Hotel_Light.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Light : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHotel_Light();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isLightOn;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isStartFlicker;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isStreetLight;

    bool isSwitchable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ULightComponent * comLight;

    FTimerHandle LightTimer;
    FTimerHandle LightTimer2;

    float fDefaultInsanity;

    void SetHardFlickering();
    void SetSoftFlickering();
    void HardFlickering();
    void SoftFlickering();
    void TurnLight(bool bOn);
    void TurnOn();
    void TurnOff();
    void SetSwitchAction(bool bOn);
    void SetFlickeringOnce();
    void FlickeringOnce();
};
