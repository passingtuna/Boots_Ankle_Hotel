// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_HangingRope.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_HangingRope : public AHotel_Object
{
	GENERATED_BODY()
    virtual void BeginPlay() override;

    void UseRope();
    UStaticMeshComponent* RopeMesh;
};
