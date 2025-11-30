// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_KeyTray.h"
#include "Hotel_Manager.h"
// Sets default values
AHotel_KeyTray::AHotel_KeyTray()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHotel_KeyTray::BeginPlay()
{
	Super::BeginPlay();
    GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->SetKeyTray(this);
}
void AHotel_KeyTray::GiveRoomKey(FName RoomNum)
{
    int32 nRoomNum = FCString::Atoi(*RoomNum.ToString());

    int Temp = ((nRoomNum - 201) / 20) + ((nRoomNum - 201) % 20); //201~204 301~304 의 방번호를 받았을때 0~7까지의 인덱스로 변환시키는 식
  
    if (0 <= Temp && Temp < arrKey.Num())
    {
        arrKey[Temp]->SetVisibility(false);
    }
}

void AHotel_KeyTray::ReturnRoomKey(FName RoomNum)
{
    int32 nRoomNum = FCString::Atoi(*RoomNum.ToString());
    int Temp = ((nRoomNum - 201) / 20) + ((nRoomNum - 201) % 20); //201~204 301~304 의 방번호를 받았을때 0~7까지의 인덱스로 변환시키는 식

    if (0 <= Temp && Temp < arrKey.Num())
    {
        arrKey[Temp]->SetVisibility(true);
    }
}