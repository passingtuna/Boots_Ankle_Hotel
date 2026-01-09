// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_Door.generated.h"

/**
 * 
 */
class UTextRenderComponent;
class UCameraComponent;
class UBoxComponent;
class USpringArmComponent;
class AHotel_Guest;
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Door : public AHotel_Object
{
	GENERATED_BODY()
protected:
    bool isOpen;
    bool isLock;
    bool isInside;
    bool isMoving;
    bool isActivePeepingFaceInside;
    bool isActivePeepingFaceOutside;
    bool isUncontrolable;
    bool isMoveByWalker;
    void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    //컴포넌트 초기화
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UTextRenderComponent* DoorNameText;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    USceneComponent* Hinge;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* DoorMesh;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PeepingHoleOutside;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    USceneComponent* PeepingHoleInside;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* PeepingCameraOutside;
    UPROPERTY( BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* PeepingCameraInside;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* PeepingArmOutside;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* PeepingArmInside;


    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PeepingFaceMeshInside;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PeepingFaceMeshOutside;
    UPROPERTY(BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    UBoxComponent* OpenTriggerBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Initialize, meta = (AllowPrivateAccess = "true"))
    FName DoorName;

    
public:
    void MovePeepingEye(FVector2D LookAxisVector);
    void PeepingEnd();
    void ToggleOpen();
    void ToggleLock();

    void ToggleOpenByWalker();
    void ToggleLockByWalker();

    void Peeping();
    void Knock(int Type = 0);
    UFUNCTION(BlueprintCallable)
    void SetDoorNameText(FName Name);
    void CheckInteractLocation();

    void SetPeepingFace(bool active , bool IsInside) { IsInside? isActivePeepingFaceInside = active : isActivePeepingFaceOutside = active;  };
    UFUNCTION(BlueprintCallable)
    void OnOpenTriggerBoxBegin(AActor* OtherActor);
    UFUNCTION(BlueprintCallable)
    void EndOpenTriggerBoxBegin(AActor* OtherActor);

    void SetLockDoor(bool state);
    void SetUncontrolableLock(bool state) { isUncontrolable = state; };
    void SetOpenDoor(bool state);
    void SetOpenInside(bool state) { isInside = state; };

    void GuestOpen(AHotel_Guest* Guest);
    void GuestClose(AHotel_Guest* Guest);
    bool GetOpenState() { return isOpen; };
    bool GetLockState() { return isLock; };
};
