// Fill out your copyright notice in the Description page of Project Settings.

#include "Hotel_Types.h"

FString HotelTriggerStateToString(EHotelObjectState State)
{
    switch (State)
    {
    case EHotelObjectState::In: return TEXT("In");
    case EHotelObjectState::Out: return TEXT("Out");
    case EHotelObjectState::Look: return TEXT("Look");
    case EHotelObjectState::NoLook: return TEXT("NoLook");
    case EHotelObjectState::On: return TEXT("On");
    case EHotelObjectState::Off: return TEXT("Off");
    case EHotelObjectState::Open: return TEXT("Open");
    case EHotelObjectState::Close: return TEXT("Close");
    case EHotelObjectState::Lock: return TEXT("Lock");
    case EHotelObjectState::UnLock: return TEXT("UnLock");
    case EHotelObjectState::Start: return TEXT("Start");
    case EHotelObjectState::End: return TEXT("End");
    case EHotelObjectState::View: return TEXT("View");
    default: return TEXT("");
    }
}

bool HotelTriggerStateEquals(const FString* Value, EHotelObjectState State)
{
    return Value && *Value == HotelTriggerStateToString(State);
}

FHotelTrigger FHotelTrigger::Make(EHotelTriggerType InType, const TMap<EHotelTriggerKey, FString>& InPayload)
{
    FHotelTrigger Out;
    Out.Type = InType;
    Out.Payload = InPayload;
    return Out;
}

bool FHotelTrigger::operator==(const FHotelTrigger& Other) const
{
    return Type == Other.Type && Payload.OrderIndependentCompareEqual(Other.Payload);
}

