// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Operator.h"
#include "Hotel_Manager.h"
#include "Hotel_Walker.h"
void AHotel_Operator::BeginPlay()
{
    Super::BeginPlay();
    Hotel_Walker = Cast<AHotel_Walker>(GetWorld()->GetFirstPlayerController()->GetPawn());
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();

    aPhone->SetPhoneWatchGuest(this);
    Hotel_Manager->SetDepartmentOperator(this);
}

UDialogueDataAsset* AHotel_Operator::GetDailogueData(EDialogueState DialogueState, bool isCall)
{
    FName DialogueName;
    switch (DialogueState)
    {
        case EDialogueState::DS_Manager_Warning:
        {
            DialogueName = "Manager_Warning_Walker";
        }
        break;

        case EDialogueState::DS_Manager_Fire:
        {
            DialogueName = "Manager_Fire_Walker";
        }
        break;
        case EDialogueState::DS_Security:
        {
            DialogueName = "SecurityTeam";
        }
        break;
        case EDialogueState::DS_Manager_Request_Reject_CheckIn:
        {
            DialogueName = "Manager_Ask_GuestDoNotEnter";
        }
        break;

        case EDialogueState::DS_Manager_Allocate_Room_Guest:
        {
            DialogueName = "Manager_Ask_AllocateRoomGuest";
        }
        break;
        default:
            DialogueName = "";
            break;
    }
    return Hotel_Manager->GetDialogueData(DialogueName);
}

void AHotel_Operator::CallingFailAction()
{
    UE_LOG(LogTemp, Warning, TEXT("호텔 오퍼레이터 컬링페일 액션"));
    FTimerHandle Timerhandle;
    GetWorld()->GetTimerManager().SetTimer(Timerhandle, [this]()
        {
            Hotel_Manager->MinusHRScore(10, TEXT("전화 대기 불량"));
        }
    , 3.0f, false); //바로 차감시 전화 종료되기전 바로 경고나 해고 전화를 걸어 또 응답실패 함수로 들어오게된다

    if (Hotel_Manager->GetWalkerFired())
    {
        Hotel_Manager->StartWalkerFireProcess();
    }
}