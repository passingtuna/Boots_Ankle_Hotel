// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Cleanable_Object.h"
#include "Hotel_Manager.h"

void AHotel_Cleanable_Object::BeginPlay()
{
    availableAction.Add(TEXT("청소"), [this]() { CleanObject(); });
    Super::BeginPlay();
    if (!isClean)
    {
        DirtObject();
    }
}


void AHotel_Cleanable_Object::CleanObject()
{
    Mesh->SetMaterial(0,NULL);
    PlaySound("Clean");
    isClean = true;
    Hotel_Manager->OnEventTriggerAction(
        FHotelTrigger::Make(
            EHotelTriggerType::RoomCleaned,
            {
                { EHotelTriggerKey::Place, RoomNumber.ToString() },
            }));
}

void AHotel_Cleanable_Object::DirtObject()
{
    Mesh->SetMaterial(0,DirtyMaterial);
    isClean = false;
}