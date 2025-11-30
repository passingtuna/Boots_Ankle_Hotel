// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_Menual.generated.h"

/**
 * 
 */
class UMenualUI;

UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Menual : public AHotel_Object
{
	GENERATED_BODY()
	
    virtual void BeginPlay() override;

    void OpenMenual();
};
