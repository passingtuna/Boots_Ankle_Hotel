// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Hotel_StaticMesh.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_StaticMesh : public UStaticMeshComponent
{
	GENERATED_BODY()
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Name")
    FName MeshName;
    void SetMeshState(bool state);
};
