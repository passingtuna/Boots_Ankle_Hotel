// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hotel_CCTV_Camera.generated.h"

class USceneCaptureComponent2D;

UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_CCTV_Camera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHotel_CCTV_Camera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
    FName CameraName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Init")
    USceneCaptureComponent2D* CameraComp;
    void SetCameraState(bool state);
    void SetPostProcess(int num , bool On);

    void SetNoiseOnceCamera(float Time);
};
