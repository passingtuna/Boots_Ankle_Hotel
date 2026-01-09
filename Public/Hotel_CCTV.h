// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_CCTV.generated.h"

/**
 * 
 */
class AHotel_CCTV_Camera;
class UTextRenderComponent;
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_CCTV : public AHotel_Object
{
	GENERATED_BODY()

private:
    int nNowCameraNum;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
    void ViewPrevCamera();
    void ViewNextCamera();
    void ViewIndexCamera(int Index);
    void SetPostProcessCamera(int CameraNum,int ProcessingNum , bool On);
    void SetHiddenObject(AActor* HiddenActor);
    int FindNextCameraIndex(bool Next);
    int FindCameraNumByName(FName Name);
    int GetNowCameraNum() { return nNowCameraNum; };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = CCTV_CAMERA, meta = (AllowPrivateAccess = "true"))
    TArray <AHotel_CCTV_Camera*> arrCameras;
    UTextRenderComponent * MoniterText;


    void ActiveRoomCameraToCCTV(AHotel_CCTV_Camera* camera);
    void EraseRoomCameraToCCTV(AHotel_CCTV_Camera* camera);
    int GetRoomCameraIndex(FName cameraname);
};