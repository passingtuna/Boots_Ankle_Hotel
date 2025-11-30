// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_Cleanable_Object.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Cleanable_Object : public AHotel_Object
{
	GENERATED_BODY()
	
protected:
    virtual void BeginPlay() override;
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "object")
    UMaterial* DirtyMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "object")
    bool isClean = true;
    FName RoomNumber;
    bool GetClean() { return isClean; };
    void CleanObject();
    void DirtObject();
    void SetRoomNumber(FName roomnum) { RoomNumber = roomnum; };
};
