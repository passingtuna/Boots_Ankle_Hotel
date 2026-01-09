// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hotel_Place.generated.h"

class AHotel_Guest;
class UBoxComponent;
class AHotel_Switch;

UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Place : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHotel_Place();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Place")
    FName PlaceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
    UBoxComponent* PlaceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Place")
    AHotel_Switch* PlaceSwtich;

    UFUNCTION()
    void PersonInPlace(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void PersonOutPlace(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void UpdateWalkerLocation();

    void SetOverlappingPerson(AActor* OtherActor);

    void AddRegistPlace();

    AActor* GetParentActor() { return GetAttachParentActor();};
 
    AHotel_Guest  * CurrentGuest;
    AHotel_Place  * ParentPlace;
};
