// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"

// Sets default values
AHotel_Guest_Room::AHotel_Guest_Room()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bGuestIn = false;
}

// Called when the game starts or when spawned
void AHotel_Guest_Room::BeginPlay()
{
    Super::BeginPlay();
    //UE_LOG(LogTemp,Warning,TEXT("비긴 플레이 %s"), *RoomNumber.ToString()); 
    GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->AddRegistedRoom(RoomNumber, this);
    MakeRoomDirty(FMath::RandRange(0, 3));
    if (RoomNumber == "205" || RoomNumber == "305")
    {
        this->SetActorHiddenInGame(true);
    }
    InitChildActorComponent();
}

void AHotel_Guest_Room::InitChildActorComponent()
{
    if (IsValid(aMainSwitch))
    {
        aMainSwitch->AddConnectedLight(aMainLight);
        aMainSwitch->AddConnectedLight(aMainLight2);
        aMainSwitch->SetSwitchName(RoomNumber);
    }
    if (IsValid(aToiletSwitch))
    {
        aToiletSwitch->AddConnectedLight(aToiletLight);
        aMainSwitch->SetSwitchName(RoomNumber);
    }
    if (IsValid(aDoor))
    {
        aDoor->SetDoorNameText(RoomNumber);
    }
    if (IsValid(aPhone))
    {
        aPhone->SetPhoneNumber(RoomNumber);
    }

    if (IsValid(aPlace_Volume))
    {
        aPlace_Volume->PlaceName = RoomNumber;
        aPlace_Volume->AddRegistPlace();
    }

    if (IsValid(aToilet_Volume))
    {
        FString ToiletName = RoomNumber.ToString() + "_Toilet";
        aToilet_Volume->PlaceName = FName(*ToiletName);
        aToilet_Volume->AddRegistPlace();
        aToilet_Volume->ParentPlace = aPlace_Volume;
    }
    if (IsValid(aCCTV_Camera))
    {
        aCCTV_Camera->CameraName = RoomNumber;
        aCCTV_Camera->SetCameraState(false);
    }

    if (IsValid(RopeStaticComponent))
    {
        RopeStaticComponent->SetVisibility(false);
    }

    if (IsValid(aBed))
    {
        aBed->SetRoomNumber(RoomNumber);
    }

    if (IsValid(aToilet))
    {
        aToilet->SetRoomNumber(RoomNumber);
    }
}
// Called every frame
void AHotel_Guest_Room::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void AHotel_Guest_Room::MakeRoomDirty(int state)
{
    switch (state)
    {
    case 0:
        break;
    case 1:
        aToilet->DirtObject();
        break;
    case 2:
        aBed->DirtObject();
        break;
    case 3:
        aToilet->DirtObject();
        aBed->DirtObject();
        break;
    }
}

bool AHotel_Guest_Room::CheckRoomDirty()
{
    if(aToilet->GetClean() && aBed->GetClean())
    {
        return true;
    }
    return false;
}
void AHotel_Guest_Room::HangingNeck(AHotel_Guest* HangedMan)
{

    UE_LOG(LogTemp, Warning, TEXT("행잉 넥"));
    if (IsValid(RopeStaticComponent))
    {
        RopeStaticComponent->SetVisibility(true);
    }
    /*
    FVector temp = RopeStaticComponent->GetComponentTransform().GetLocation();
    HangedMan->SetHangingState(true);
    temp.Y -= 5;
    temp.Z -= 80;
    */
    HangedMan->SetHangingState(true);
    FVector relativeOffset(0, -5, -80);
    FVector worldPos = RopeStaticComponent->GetComponentTransform().TransformPosition(relativeOffset);
    HangedMan->TeleportTo(worldPos, RopeStaticComponent->GetComponentRotation());

    UE_LOG(LogTemp, Warning, TEXT("행잉 넥 %f : %f : %f") , worldPos.X, worldPos.Y, worldPos.Z);
    HangedMan->TeleportTo(worldPos, RopeStaticComponent->GetComponentRotation());
}

void AHotel_Guest_Room::CheckOutProcess()
{
    if (IsValid(RopeStaticComponent))
    {
        RopeStaticComponent->SetVisibility(false);
    }
    MakeRoomDirty(FMath::RandRange(0, 3));
}


void AHotel_Guest_Room::SetRoomFlickingLight(int mode)
{
    switch(mode)
    {
    case 1:
    {
        if (IsValid(aMainLight))
        {
            aMainLight->SetSoftFlickering();
        }
        if (IsValid(aMainLight2))
        {
            aMainLight2->SetSoftFlickering();
        }
    }
        break;
    case 2:
    {
        if (IsValid(aMainLight))
        {
            aMainLight->SetHardFlickering();
        }
        if (IsValid(aMainLight2))
        {
            aMainLight2->SetHardFlickering();
        }

    }
        break;
    }
}

void AHotel_Guest_Room::SetIsolationState(bool on)
{
    if (IsValid(aDoor)) aDoor->SetIsInteractive(!on);
    if (IsValid(aPhone)) aPhone->SetIsInteractive(!on);
}
