// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Clock.h"
#include "Hotel_Manager.h"
#include "Blueprint/UserWidget.h"



void AHotel_Clock::BeginPlay()
{
    Super::BeginPlay();
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    Hotel_Manager->SetHotelClock(this);
    nEnviromentalLevle = Hotel_Manager->GetEnviromentalLevel();
}

void AHotel_Clock::Tick(float DeltaTime)
{
    int nHour = nGameTime >= 1440 ? (nGameTime - 1440) / 60 : nGameTime / 60;
    int nMinute =  nGameTime % 60;
    HourNeedleComp->SetRelativeRotation(FRotator(0 , nHour * 30 + (nMinute / 2.0f), 0));
    MinuteNeedleComp->SetRelativeRotation(FRotator(0 , nMinute * 6 , 0));
    InteractMassage = FString::FromInt(nHour) + TEXT("시 ") + FString::FromInt(nMinute) + TEXT("분");
}

void AHotel_Clock::AddMinute()
{
    if (bIsStopTime) return;

    nGameTime +=1;
    if (nGameTime >= 1800) //6시 게임끝
    {                   
        Hotel_Manager->SetGameEnd(EGameEndReason::GER_Clear);
        nGameTime = 0;
    }

    if (nGameTime >= nNextEventTime)
    {
        Hotel_Manager->AddNextExecutiongEventList();
        if (FMath::RandRange(0, 100) > 10)
        {
            switch (nEnviromentalLevle)
            {
            case 0:
                nNextEventTime = nGameTime + FMath::RandRange(40, 60);
                break;
            case 1:
                nNextEventTime = nGameTime + FMath::RandRange(30, 50);
                break;
            case 2:
                nNextEventTime = nGameTime + FMath::RandRange(20, 30);
                break;
            }
        }
        else
        {
            UE_LOG(LogTemp,Warning,TEXT("2명 동시 입장"));
        }
    }
}

void AHotel_Clock::StartGameClock()
{
    GetWorld()->GetTimerManager().SetTimer(ClockTimer, this, &AHotel_Clock::AddMinute, 1.0, true);//2초에 1분씩
    nGameTime = 1290;//게임시작 21시30분 
    nNextEventTime = 1290 + 30 + FMath::RandRange(0,20); //10시~10시 20분 사이 랜덤으로 첫 이벤트 시작

    nGameTime = nNextEventTime;
}


void AHotel_Clock::SetHighLightInteractive(bool OnOff) //시계는 하이라이트 하지않는다
{

}
