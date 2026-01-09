// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hotel_Guest_Room.generated.h"

class AHotel_Bed;
class AHotel_Toilet;
class AHotel_Switch;
class AHotel_Toilet_Door;
class AHotel_CCTV_Camera;
class AHotel_Light;
class AHotel_Phone;
class AHotel_Place;
class AHotel_Door;
class AHotel_Guest;

UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Guest_Room : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHotel_Guest_Room();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    bool bGuestIn;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Room")
    FName RoomNumber;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Phone * aPhone;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Bed * aBed;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Toilet * aToilet;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Switch * aMainSwitch;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Switch * aToiletSwitch;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Light * aMainLight;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Light* aMainLight2;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Light * aToiletLight;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Door * aDoor;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Toilet_Door * aToilet_Door;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Place * aPlace_Volume;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_Place * aToilet_Volume;
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object", meta = (ExposeOnSpawn = true))
    UStaticMeshComponent* RopeStaticComponent;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Object")
    AHotel_CCTV_Camera* aCCTV_Camera;

    UFUNCTION(BlueprintCallable)
    void InitChildActorComponent();
    FName GetRoomNumber() { return RoomNumber; };
    void SetGuestIn(bool value) { bGuestIn = value; };
    void MakeRoomDirty(int state);
    void HangingNeck(AHotel_Guest* HangedMan);
    bool CheckRoomDirty();
    void CheckOutProcess();
    void SetRoomFlickingLight(int mode);

    void SetIsolationState(bool on);
};
