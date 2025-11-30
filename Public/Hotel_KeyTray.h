// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hotel_KeyTray.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_KeyTray : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHotel_KeyTray();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Initialize)
    TArray<UStaticMeshComponent*> arrKey;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = Initialize)
    UStaticMeshComponent* Key201;


    void GiveRoomKey(FName RoomNum);
    void ReturnRoomKey(FName RoomNum);


};
