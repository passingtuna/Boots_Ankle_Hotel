// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Phone.h"
#include "PhoneDialUI.h"
#include "Hotel_Manager.h"
#include "Hotel_Walker.h"

void AHotel_Phone::BeginPlay()
{
    ObjectName = TEXT("전화기");
    availableAction.Add(TEXT("수화기 들기"), [this]() { PickReceiver(); });
    QuickActionName = TEXT("수화기 들기");
    Super::BeginPlay();
    if (RegistPhoneNumber != "")
    {
        Hotel_Manager->AddRegistPhone(RegistPhoneNumber, this);
    }
    NowCallingPhoneNum = NAME_None;
    aConnectedPhone = NULL;
    isUserPickUpPhone = false;
    IsDisconnect = false;
}

void AHotel_Phone::SetPhoneNumber(FName PhoneNum)
{
    RegistPhoneNumber = PhoneNum;
    Hotel_Manager->AddRegistPhone(RegistPhoneNumber, this);
}
void AHotel_Phone::PickReceiver()
{
    isUserPickUpPhone = true;
    IsDisconnect = false;
    if (aConnectedPhone) //수화기를 들었을때 연결된 전화가 있을경우 = 전화벨이 울리고있을때
    {
        RecieveCalling();
    }
    else
    {
        AudioComp->OnAudioFinished.AddDynamic(this, &AHotel_Phone::PlayToneDial);
        PlaySound("PickUp");
        
    }

    if (Hotel_Walker)
    {
        Hotel_Walker->ViewUIPhoneDial(this);
    }
}

void AHotel_Phone::PlayToneDial()
{
    AudioComp->OnAudioFinished.Clear();
    PlaySound("ToneDial");
}
void AHotel_Phone::TryCalling(FName PhoneNum)
{
    PlaySound("TryConnecting");
    NowCallingPhoneNum = PhoneNum;
    Hotel_Manager->TryCalling(this, NowCallingPhoneNum);
}

void AHotel_Phone::RingingTimeOver()
{
    UE_LOG(LogTemp, Warning, TEXT("링 오버 타임 : %s"), *RegistPhoneNumber.ToString());

    if (IsValid(aConnectedPhone))
    {
        if (IsValid(aConnectedPhone->PhoneWatchGuest))
        {
            aConnectedPhone->PhoneWatchGuest->CallingFailAction();
        }

        aConnectedPhone->ConnectFail();
    }
    ConnectFail();
    StopSound();
}
void AHotel_Phone::ConnectFail()
{
    if (IsValid(Hotel_Walker) && Hotel_Walker->IsInteractThisObject(this))
    {
        UE_LOG(LogTemp, Warning, TEXT("디스커넥트 플레이"), *RegistPhoneNumber.ToString());
        PlaySound("Disconnect");
    }
    else
    {
        StopSound();
    }
    aConnectedPhone = NULL;
    NowCallingPhoneNum = NAME_None;
    IsDisconnect = true;

    UE_LOG(LogTemp, Warning, TEXT("커넥트 페일 : %s"), *RegistPhoneNumber.ToString());
    GetWorld()->GetTimerManager().ClearTimer(PhoneTimer);
}

void AHotel_Phone::ConnectTry(AHotel_Phone* ConnectingPhone)
{
    aConnectedPhone = ConnectingPhone;
    UE_LOG(LogTemp, Warning, TEXT("커넥트 트라이 : %s"), *RegistPhoneNumber.ToString());
    if (NowCallingPhoneNum.IsNone()) //전화 걸고 있는 쪽이 아니라면
    {
        UE_LOG(LogTemp, Warning, TEXT("링잉벨 : %s"), *RegistPhoneNumber.ToString());
        PlaySound("Ring"); //벨을 울린다
        if (!isUserPickUpPhone && IsValid(PhoneWatchGuest)) //유저가 들고 있는 폰이 아닌데 게스트가 할당된 전화기라면 게스트가 전화 받음
        {
            int TempTime = FMath::RandRange(4,10); //4~10초뒤에 전화 받기
            GetWorld()->GetTimerManager().SetTimer(PhoneTimer, this, &AHotel_Phone::RecieveCalling, TempTime, false);
        }
        else //그외엔 20초간 벨울리기
        {
            GetWorld()->GetTimerManager().SetTimer(PhoneTimer, this, &AHotel_Phone::RingingTimeOver, 15.0, false); //15초동안 응답 없다면 연결끊김
        }
    }
}

void AHotel_Phone::RecieveCalling()
{
    if (!IsInteractable)        //해당 전화가 상호작용 불가능 상태시 연결 성공대신 리턴
    {
        RingingTimeOver();
        return;
    }

    if (aConnectedPhone)
    {
        ConnectSuccess();
        aConnectedPhone->ConnectSuccess();
    }
}

void AHotel_Phone::ConnectSuccess()
{
    StopSound();
    GetWorld()->GetTimerManager().ClearTimer(PhoneTimer);

    //대화 시작
    if (isUserPickUpPhone && IsValid(aConnectedPhone) && IsValid(aConnectedPhone->PhoneWatchGuest))//유저가 들고 있는 폰쪽에서만 실행
    {
        Hotel_Walker->ViewUIDialogue(aConnectedPhone->PhoneWatchGuest ,this);
    }
}

void AHotel_Phone::EndPhoneUse()
{
    if (aConnectedPhone)
    {
        aConnectedPhone->ConnectFail();
        aConnectedPhone->aConnectedPhone = NULL;
        aConnectedPhone->GetWorld()->GetTimerManager().ClearTimer(PhoneTimer);
    }
    if (Hotel_Walker)
    {
        Hotel_Walker->HideUIName("Phone");
        Hotel_Walker->GetMouseControl();
    }
    StopSound();
    PlaySound("PutDown");
    aConnectedPhone = NULL;
    isUserPickUpPhone = false;
    IsDisconnect = false;
    NowCallingPhoneNum = NAME_None;
    GetWorld()->GetTimerManager().ClearTimer(PhoneTimer);
}