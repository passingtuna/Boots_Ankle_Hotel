// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GuestFaceAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API UGuestFaceAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    bool IsHanging;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    bool IsNeckShaking;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    bool IsLaughing;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* IdleAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* HangingAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* NeckShakingAnim;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Anim")
    UAnimSequence* LaughAnim;
	
};
