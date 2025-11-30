// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_HangingRope.h"


void AHotel_HangingRope::BeginPlay()
{
    ObjectName = TEXT("로프");
    availableAction.Add(TEXT("사용"), [this]() { UseRope(); });

    Super::BeginPlay();
}

void AHotel_HangingRope::UseRope()
{

}
