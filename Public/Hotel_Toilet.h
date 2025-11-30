// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Cleanable_Object.h"
#include "Hotel_Toilet.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Toilet : public AHotel_Cleanable_Object
{
	GENERATED_BODY()
    virtual void BeginPlay() override;

};
