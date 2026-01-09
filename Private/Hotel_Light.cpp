// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Light.h"
#include "Hotel_Manager.h"
#include "Components/LightComponent.h"

// Sets default values
AHotel_Light::AHotel_Light()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHotel_Light::BeginPlay()
{
	Super::BeginPlay();
    fDefaultInsanity = comLight->Intensity;
    isSwitchable = true;
    TurnLight(isLightOn); //시작시 조명 켜진상태인지 꺼진상태인지
    if (isStartFlicker)
    {
        SetSoftFlickering();
    }

    if (isStreetLight)
    {
        GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->SetStreetLight(this);
    }
}

// Called every frame
void AHotel_Light::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHotel_Light::SetSoftFlickering()
{
    GetWorld()->GetTimerManager().SetTimer(LightTimer, this, &AHotel_Light::SoftFlickering, 0.1f);
}
void AHotel_Light::SetHardFlickering()
{
    GetWorld()->GetTimerManager().SetTimer(LightTimer, this, &AHotel_Light::HardFlickering, 0.1f,true);
}
void AHotel_Light::SetFlickeringOnce()
{
    TurnOff();
    GetWorld()->GetTimerManager().SetTimer(LightTimer, this, &AHotel_Light::FlickeringOnce, 0.5f);
}

void AHotel_Light::SoftFlickering()
{
    int TempIntensity = FMath::RandRange(1,5);
    comLight->SetIntensity(fDefaultInsanity * 0.2f * TempIntensity);
    float TempTime = FMath::RandRange(0.001f, 0.2f);
    GetWorld()->GetTimerManager().SetTimer(LightTimer, this, &AHotel_Light::SoftFlickering, TempTime);
}

void AHotel_Light::HardFlickering()
{
     TurnLight(!isLightOn);
}

void AHotel_Light::FlickeringOnce()
{
    GetWorld()->GetTimerManager().ClearTimer(LightTimer2);
    TurnOn();
}


void AHotel_Light::TurnLight(bool bOn)
{
    if (!isSwitchable) return;
    isLightOn = bOn;
    bOn ? TurnOn() : TurnOff();
}

void AHotel_Light::TurnOn()
{
    isLightOn = true;
    comLight->SetIntensity(fDefaultInsanity);
}

void AHotel_Light::TurnOff()
{
    isLightOn = false;
    comLight->SetIntensity(0);
}

void AHotel_Light::SetSwitchAction(bool bOn)
{   //스위치로 키면 정상화
    if (!bOn)//스위치로 끄면 깜빡임끄기
    {
        GetWorld()->GetTimerManager().ClearTimer(LightTimer);
        GetWorld()->GetTimerManager().ClearTimer(LightTimer2);
    }
    TurnLight(bOn);
}
