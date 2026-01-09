// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Hotel_Guest_Default.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationPath.h" 
#include "Hotel_Guest.h"
#include "Hotel_Door.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"
#include "Hotel_Place.h"
#include "Hotel_Phone.h"
#include "Hotel_Bed.h"

void AAI_Hotel_Guest_Default::BeginPlay()
{

    Super::BeginPlay();
}


void AAI_Hotel_Guest_Default::InitAIController(AHotel_Guest * guest)
{
    Hotel_Guest = guest;
    Possess(Hotel_Guest);
    Hotel_Guest->SetAIController(this);
    if (!ReceiveMoveCompleted.IsAlreadyBound(this, &AAI_Hotel_Guest_Default::OnMoveCompletedCallback))
    {
        ReceiveMoveCompleted.AddDynamic(this, &AAI_Hotel_Guest_Default::OnMoveCompletedCallback);
    }
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    MoveSuccessState = AS_None;
    nPatience = FMath::RandRange(3, 5); //인내심 카운터 2~5 랜덤 설정
    if (IsValid(Hotel_Guest))
    {
        Hotel_Guest->AddDialogueDataState(TEXT("체크 인에 관하여"), EDialogueState::DS_Guest_CheckIn);
    }
}
void AAI_Hotel_Guest_Default::AssigningGuestRoom(AHotel_Guest_Room* GuestRoom)
{

    if (IsValid(Hotel_Guest))
    {
        AssignedGuestRoom = GuestRoom;
        MoveSuccessState = AS_EndMoveRoom;
        Hotel_Guest->CheckInRoomNum = GuestRoom->RoomNumber;
        GoToRoom();
    }
}
void AAI_Hotel_Guest_Default::GoToRoomDoor()
{
    if (IsValid(AssignedGuestRoom))
    {
        if (IsValid(Hotel_Guest))
        {
            Hotel_Guest->ReadyToMove();
            FVector DoorLocation = AssignedGuestRoom->aDoor->GetActorLocation();
            FVector MoveTarget = DoorLocation - AssignedGuestRoom->aDoor->GetActorRightVector() * 120.0f; // 문 앞 약간 떨어진 지점
            MoveToLocation(MoveTarget, 5.0f);
        }
    }
}
void AAI_Hotel_Guest_Default::GoToCounter()
{
    if (IsValid(Hotel_Guest))
    {
        FVector TargetLocation = FVector(-1139, 2063, 96);
        Hotel_Guest->ReadyToMove();
        MoveToTargetLocation(TargetLocation);
        MoveSuccessState = AS_EndMoveCounter;
    }
}
void AAI_Hotel_Guest_Default::GoToRoom()
{
    if (IsValid(Hotel_Guest))
    {
        Hotel_Guest->SetAutoActionDoor(true);
        Hotel_Guest->ReadyToMove();
        if (IsValid(AssignedGuestRoom))
        {
            MoveToActor(AssignedGuestRoom->aBed, 10);
        }
    }
}
void AAI_Hotel_Guest_Default::HearingKnock()
{
    if (IsValid(Hotel_Guest))
    {
        Hotel_Guest->ReadyToMove();
        Hotel_Guest->SetAutoActionDoor(false);
        GoToRoomDoor();
        MoveSuccessState = AS_EndHearingKnock;
    }
}

void AAI_Hotel_Guest_Default::GoToStaffPlace()
{
    if (IsValid(Hotel_Guest))
    {
        Hotel_Guest->SetAutoActionDoor(true);
        Hotel_Guest->ReadyToMove();
        FVector TargetLocation = FVector(-971, 2650, 94);
        MoveToTargetLocation(TargetLocation);
    }
}
void AAI_Hotel_Guest_Default::GoToOutside()
{
    if (IsValid(Hotel_Guest))
    {
        Hotel_Guest->SetAutoActionDoor(true);
        Hotel_Guest->ReadyToMove();

        MoveSuccessState = AS_OutSide;
        FVector TargetLocation = FVector(-327, 839, 94);
        MoveToTargetLocation(TargetLocation);
    }
}

void AAI_Hotel_Guest_Default::MoveToTargetLocation(FVector TargetLocation)
{
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    FNavLocation Projected;

    if (NavSys && NavSys->ProjectPointToNavigation(TargetLocation, Projected))
    {
        MoveToLocation(Projected.Location, 20.f);
    }
}
void AAI_Hotel_Guest_Default::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAI_Hotel_Guest_Default::OnMoveCompletedCallback(FAIRequestID RequestID, const EPathFollowingResult::Type Result)
{
    FString ResultStr;

    //UE_LOG(LogTemp, Warning, TEXT("온 무브 컴플릿 콜백 %d : %d") , Result, MoveSuccessState);
    switch (Result)
    {
        case EPathFollowingResult::Success:
            {
                Hotel_Guest->SetAutoActionDoor(true);
                switch (MoveSuccessState)
                {
                case AS_None://이동 상태가 아무것도 아닐시 기본 대화 세팅
                    {
                        
                    }
                    break;
                case AS_EndMoveCounter:
                    {
                        Hotel_Guest->SetLookingPlayer(true);
                        if (Hotel_Manager->GetWalkerLocation() == "Counter") //
                        {
                            UE_LOG(LogTemp, Warning, TEXT("카운터에 있는데 말을 안걸어서 일단 해버림"));
                            GetWorld()->GetTimerManager().SetTimer(Timer_Patience, [this]() {DecreasePatienceCount(true); }, 15 , false);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("셋 링잉 벨 타이머"));
                            GetWorld()->GetTimerManager().SetTimer(Timer_AI, this, &AAI_Hotel_Guest_Default::RingingBell, 8);//10초 대기후 대기벨 울리기
                        }
                    }
                    break;
                case AS_EndMoveRoom:
                    if (IsValid(AssignedGuestRoom))
                    {
                        AssignedGuestRoom->SetGuestIn(true);
                        CheckingRoomCondition();
                    }
                    break;
                case AS_EndHearingKnock:
                    {
                        Hotel_Guest->ArrivedKnockingDoor(AssignedGuestRoom->aDoor);
                        MoveSuccessState = 0;
                        Hotel_Guest->SetAutoActionDoor(false);
                    }
                    break;
                case AS_LockingDoor:
                    {
                        if (IsValid(AssignedGuestRoom))
                        {
                            AssignedGuestRoom->aDoor->SetOpenDoor(false);
                            AssignedGuestRoom->aDoor->SetLockDoor(true);
                            GetWorld()->GetTimerManager().SetTimer(Timer_AI, [this]() {
                                Hotel_Guest->ReadyToMove();
                                GoToRoom();
                                }, 1.0f, false);

                        }
                    }
                    break;
                case AS_OutSide:
                    {
                        if (Hotel_Guest->IsCheckTrigger) Hotel_Manager->OnEventTriggerAction(FName(Hotel_Guest->GuestName + "_OutHotel"));
                        Hotel_Manager->GuestOutHotel(Hotel_Guest); //실행중 이벤트에서 빼고
                        Hotel_Guest->CheckOutGuest();//체크인된 방이 있다면 체크아웃
                        Hotel_Guest->DeactivateGuest();
                    }
                    break;
                }
                MoveSuccessState = AS_None;
            }
        break;
        case EPathFollowingResult::Blocked:
            {
                ResultStr = TEXT("Blocked (Path interrupted or obstacle)");

                GetWorld()->GetTimerManager().SetTimer(Timer_AI, [this]()
                    {
                        switch (MoveSuccessState)
                        {
                            case AS_EndMoveCounter:
                            {
                                GoToCounter();
                            }
                            break;
                            case AS_EndMoveRoom:
                            {
                                GoToRoom();
                            }
                            break;
                            case AS_EndHearingKnock:
                            {
                                GoToRoomDoor();
                            }
                            break;
                            case AS_LockingDoor:
                            {
                                GoToRoomDoor();
                            }
                            break;
                            default:
                            {
                            }
                            break;
                        }
                    }, 2, false);

            }
        break;
        case EPathFollowingResult::OffPath:
        {
            ResultStr = TEXT("OffPath");
        }
        break;
        case EPathFollowingResult::Aborted:
        {
            ResultStr = TEXT("Aborted (Movement cancelled)");
        }
        break;
        case EPathFollowingResult::Invalid:
        {
            ResultStr = TEXT("Invalid path");
        }
        break;
        default:
        {
            ResultStr = TEXT("Unknown");
        }
        break;
    }

}

void AAI_Hotel_Guest_Default::CheckingRoomCondition()
{
    if (IsValid(AssignedGuestRoom))
    {
        AssignedGuestRoom->aDoor->SetLockDoor(true);
        
        if (IsValid(Hotel_Guest))
        {
            AssignedGuestRoom->aPhone->SetPhoneWatchGuest(Hotel_Guest);  //방에 입장시 현재 방의 전화대기

            if (!AssignedGuestRoom->CheckRoomDirty()) //방이 더러우면 전화로 컴플레인한다
            {
                Hotel_Manager->AddOutbreakEventList(Hotel_Guest, FName("ComplainRoomDirty"));
            }
        }
    }
}


void AAI_Hotel_Guest_Default::CallingFail()
{
    if (nPatience > 0 && MoveSuccessState == AS_None) //가만히 있고 인내심 바닥이 아닐경우
    {
        DecreasePatienceCount(false);
        GetWorld()->GetTimerManager().SetTimer(Timer_AI, this, &AAI_Hotel_Guest_Default::CallingWalker, 5);//10초 대기후 대기벨 울리기
    }
}

void AAI_Hotel_Guest_Default::DecreasePatienceCount(bool Looping)
{
    UE_LOG(LogTemp, Warning, TEXT("DecreasePatienceCount"));

    nPatience--;
    if (nPatience <= 0) //화나서 나갈경우
    {
        Hotel_Manager->MinusHRScore(30, TEXT("손님 접대 불량"));
        if(Hotel_Guest)Hotel_Guest->GuestExit();
        return;
    }

    if (Looping)
    {
        GetWorld()->GetTimerManager().SetTimer(Timer_Patience, [this]() 
            {
                DecreasePatienceCount(true); 
            }, 15, false);
    }
}

void AAI_Hotel_Guest_Default::CallingWalker()
{
    if (nPatience > 0)
    {
        AssignedGuestRoom->aPhone->TryCalling("0"); //같은 내용으로 다시 전화 걸기
    }
}
void AAI_Hotel_Guest_Default::CloseRoomDoor(float time)
{
    if (IsValid(AssignedGuestRoom) && AssignedGuestRoom->aDoor->GetOpenState())
    {
        GetWorld()->GetTimerManager().SetTimer(Timer_AI, [this]() {
            Hotel_Guest->ReadyToMove();
            Hotel_Guest->SetAutoActionDoor(false);
            GoToRoomDoor();
            MoveSuccessState = AS_LockingDoor;
            }, time, false);
    }
    else
    {
        return;
    }
}
void AAI_Hotel_Guest_Default::RingingBell()
{
    UE_LOG(LogTemp, Warning, TEXT("링잉 벨"));
    Hotel_Manager->RingingBell();
    GetWorld()->GetTimerManager().SetTimer(Timer_Patience, [this]() {DecreasePatienceCount(true); },15, false);
}

void AAI_Hotel_Guest_Default::StopPatienceTimer()
{
    GetWorld()->GetTimerManager().ClearTimer(Timer_Patience);
}

void AAI_Hotel_Guest_Default::StopAITimer()
{
    GetWorld()->GetTimerManager().ClearTimer(Timer_AI);
}

bool AAI_Hotel_Guest_Default::IsMovingToTarget()
{
    EPathFollowingStatus::Type Status = GetMoveStatus();

    if (Status == EPathFollowingStatus::Moving)
    {
        return true;
    }
    return false;
}

/*
void AAI_Hotel_Guest_Default::StartMove(const FVector& TargetLocation)
{
    SavedMoveLocation = TargetLocation;
    bWasMoving = true;
    MoveToLocation(TargetLocation);
}
void AAI_Hotel_Guest_Default::PauseMove()
{
    if (GetMoveStatus() == EPathFollowingStatus::Moving)
    {
        StopMovement();
        bWasMoving = true;
    }
}
void AAI_HotelGuestController::ResumeMove()
{
    if (bWasMoving)
    {
        MoveToLocation(SavedMoveLocation);
        bWasMoving = false;
    }
}*/