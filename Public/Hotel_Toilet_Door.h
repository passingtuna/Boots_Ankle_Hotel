// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Door.h"
#include "Hotel_Toilet_Door.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Toilet_Door : public AHotel_Door
{
	GENERATED_BODY()
    virtual void BeginPlay() override;
    void Tick(float DeltaTime) override;
};
