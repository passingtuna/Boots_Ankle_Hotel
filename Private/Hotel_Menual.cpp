// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Menual.h"
#include "MenualUI.h"
#include "Hotel_Walker.h"
void AHotel_Menual::BeginPlay()
{
    ObjectName = TEXT("메뉴얼");
    availableAction.Add(TEXT("열기"), [this](){OpenMenual(); });
    QuickActionName = TEXT("열기");
    Super::BeginPlay();
}


void AHotel_Menual::OpenMenual()
{
    Hotel_Walker->ViewUIMenual();
}