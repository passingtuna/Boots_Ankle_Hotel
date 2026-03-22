// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Toilet_Door.h"



void AHotel_Toilet_Door::BeginPlay()
{
    DoorName = TEXT("화장실");
    Super::BeginPlay();
}
void AHotel_Toilet_Door :: Tick(float DeltaTime)
{
    if (!isMoving) return;
    if (isOpen)
    {
        if (Hinge->GetRelativeLocation().X > -140)
        {
            Hinge->SetRelativeLocation(FVector(Hinge->GetRelativeLocation().X - 6.0f, 0.0f, 0.0f));
        }
        else
        {
            Hinge->SetRelativeLocation(FVector(-140.0f, 0.0f, 0.0f));
            SetDoorMovementActive(false);
        }
    }
    else
    {
        if (Hinge->GetRelativeLocation().X < 0)
        {
            Hinge->SetRelativeLocation(FVector(Hinge->GetRelativeLocation().X + 6.0f, 0.0f, 0.0f));
        }
        else
        {
            Hinge->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
            SetDoorMovementActive(false);
        }
    }
}