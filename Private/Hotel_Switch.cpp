// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Switch.h"
#include "Hotel_Manager.h"
#include "Hotel_Light.h"

void AHotel_Switch::BeginPlay()
{
    ObjectName = TEXT("스위치");
    availableAction.Add(TEXT("스위치"), [this]() {SetSwitchToggle();});
    QuickActionName = TEXT("스위치");
    isSwitchOn = true;
    Super::BeginPlay();

    if (SwitchName != "")
    {
        Hotel_Manager->AddHotelSwitch(SwitchName,this);
    }
}

void AHotel_Switch::SetSwitchName(FName name) 
{
    SwitchName = name; 
    Hotel_Manager->AddHotelSwitch(SwitchName,this);
}

void  AHotel_Switch::SetSwitchToggle()
{
    isSwitchOn = !isSwitchOn;
    isSwitchOn ? PlaySound("On") : PlaySound("Off");

    comSwitchButton->SetRelativeRotation(FRotator(0, 0, (180 * isSwitchOn)));

    FString temp = SwitchName.ToString() + "_Switch_" + (isSwitchOn ? "On" : "Off");

    Hotel_Manager->OnEventTriggerAction("");
    for (const auto& targetLight : arrConnectedLight)
    {
        targetLight->SetSwitchAction(isSwitchOn);// TurnLight(isSwitchOn);
    }
}

void AHotel_Switch::AddConnectedLight(AHotel_Light* AddLight)
{
    if (IsValid(AddLight))
    {
        arrConnectedLight.Add(AddLight);
    }
}
void AHotel_Switch::EventLightAction(FName nActionType)
{
    for (auto& targetLight : arrConnectedLight)
    {
        if (nActionType == "FlickingOnce")
        {
            targetLight->SetFlickeringOnce();
        }
        else if (nActionType == "FlickingHard")
        {
            targetLight->SetHardFlickering();
        }
        else if (nActionType == "FlickingSoft")
        {
            targetLight->SetSoftFlickering();
        }
        else if (nActionType == "TurnOff")
        {
            targetLight->TurnOff();
        }
        else if (nActionType == "TurnOn")
        {
            targetLight->TurnOn();
        }
    }
}