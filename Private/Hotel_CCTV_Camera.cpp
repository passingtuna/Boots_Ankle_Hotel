// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_CCTV_Camera.h"

// Sets default values
AHotel_CCTV_Camera::AHotel_CCTV_Camera()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHotel_CCTV_Camera::BeginPlay()
{
	Super::BeginPlay();
    CameraComp = FindComponentByClass<USceneCaptureComponent2D>();
    if (IsValid(CameraComp))
    {
        SetCameraState(false);
    }

    if (CameraComp->PostProcessSettings.WeightedBlendables.Array.IsValidIndex(0))
    {
        CameraComp->PostProcessSettings.WeightedBlendables.Array[0].Weight = 1;
    }
    SetCameraState(false);
}

// Called every frame
void AHotel_CCTV_Camera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHotel_CCTV_Camera::SetCameraState(bool state)
{
    if (IsValid(CameraComp))
    {
        CameraComp->SetActive(true);
        CameraComp->SetVisibility(state);
        CameraComp->bCaptureEveryFrame = state;
    }
}
void AHotel_CCTV_Camera::SetPostProcess(int num, bool On)
{
    if (CameraComp->PostProcessSettings.WeightedBlendables.Array.IsValidIndex(num))
    {
        CameraComp->PostProcessSettings.WeightedBlendables.Array[num].Weight = On;
    }
}


void AHotel_CCTV_Camera::SetNoiseOnceCamera(float Time)
{
    FTimerHandle handle;
    SetPostProcess(2, true);
    GetWorld()->GetTimerManager().SetTimer(handle, [this]() {SetPostProcess(2, false); }, Time, false);
}