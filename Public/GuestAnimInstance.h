// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GuestAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API UGuestAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    bool IsHanging;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    bool IsNeckShaking;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    int nCameraLookState = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* IdleAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* WalkAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* RunAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* HangingAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* NeckShakingAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* LookCameraAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* HoldCameraAnim;

};
