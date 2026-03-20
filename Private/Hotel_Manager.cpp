// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Manager.h"
#include "Hotel_Phone.h"
#include "Hotel_Walker.h"
#include "Hotel_Guest.h"
#include "Hotel_Place.h"
#include "Hotel_KeyTray.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Clock.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Algo/RandomShuffle.h"
#include "HotelSaveGame.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Level_Manager.h"
#include "Hotel_Switch.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI_Hotel_Guest_Default.h"
#include "DialogueDataAsset.h"
#include "GuestDataAsset.h"
#include "Hotel_Operator.h"
#include "Hotel_CCTV.h"
#include "Hotel_Light.h"
#include "Hotel_StaticMesh.h"
#include "Hotel_Door.h"
#include "Hotel_CCTV_Camera.h"
#include "Camera/CameraComponent.h"
#include "MainMenuUI.h"
#include "Hotel_PersistenceService.h"
#include "Hotel_HRService.h"
#include "Hotel_EventBase.h"
#include "Hotel_Event_Invader.h"
#include "Hotel_Event_StareUnderLight.h"
#include "Hotel_Event_GuestRoomCCTV.h"
#include "Hotel_Event_GuestHanging.h"
#include "Hotel_Event_Open205.h"
#include "Hotel_Event_GuestLostSignalCCTV.h"
#include "Hotel_Event_GuestInvisibleCamera.h"
#include "Hotel_OutbreakEventBase.h"
#include "Hotel_Outbreak_ComplainRoomDirty.h"
#include "Hotel_Outbreak_RequestRejectCheckIn.h"
#include "Hotel_Outbreak_AllocateRoomGuest.h"
#include "Hotel_Outbreak_MakeDirtyRoom.h"
#include "Hotel_Outbreak_ImpostorRequestRejectCheckIn.h"

void UHotel_Manager::Initialize(FSubsystemCollectionBase& Collection)
{
    if (!PersistenceService)
    {
        PersistenceService = NewObject<UHotel_PersistenceService>(this);
        PersistenceService->Init();
    }
    if (!HRService)
    {
        HRService = NewObject<UHotel_HRService>(this);
        HRService->ResetForNewGame();
    }

    if (EventObjects.IsEmpty())
    {
        EventObjects.Add(NewObject<UHotel_Event_Invader>(this));
        EventObjects.Add(NewObject<UHotel_Event_StareUnderLight>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestRoomCCTV>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestHanging>(this));
        EventObjects.Add(NewObject<UHotel_Event_Open205>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestLostSignalCCTV>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestInvisibleCamera>(this));
    }

    if (OutbreakEventObjects.IsEmpty())
    {
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_ComplainRoomDirty>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_RequestRejectCheckIn>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_AllocateRoomGuest>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_MakeDirtyRoom>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_ImpostorRequestRejectCheckIn>(this));
    }

    TArray<FPrimaryAssetId> AssetIDs;
    UAssetManager::Get().GetPrimaryAssetIdList(FPrimaryAssetType("DialogueDataAsset"), AssetIDs);

    for (const FPrimaryAssetId& ID : AssetIDs)
    {
        FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ID);
        UDialogueDataAsset* Asset = Cast<UDialogueDataAsset>(AssetPath.TryLoad());
        if (Asset)
        {
            FName AssetName = FName(*Asset->GetName());
            DialogueMap.Add(AssetName, Asset);
        }
    }

    AssetIDs.Empty();
    UAssetManager::Get().GetPrimaryAssetIdList(FPrimaryAssetType("GuestDataAsset"), AssetIDs);
    for (const FPrimaryAssetId& ID : AssetIDs)
    {
        FSoftObjectPath AssetPath = UAssetManager::Get().GetPrimaryAssetPath(ID);
        UGuestDataAsset* Asset = Cast<UGuestDataAsset>(AssetPath.TryLoad());
        if (Asset)
        {
            arrBPGuestDataAsset.Add(Asset);
        }
    }
    
    arrEventFuntionList.Empty();
    for (UHotel_EventBase* Evt : EventObjects)
    {
        if (!Evt) continue;
        if (Evt->EventID == EHotelEventId::None) continue;
        arrEventFuntionList.Add(Evt);
    }
    SettingInitGame(false);
}
FGuestname UHotel_Manager::GetGenderName(bool isMan)
{
    FGuestname tempGuestName;
    for (auto& temp : arrGuestName)
    {
        if (temp.isAssigned) continue;
        if (temp.isMan == isMan)
        {
            temp.isAssigned = true;
            tempGuestName = temp;
            break;
        }
    }
    return tempGuestName;
}

FString UHotel_Manager::GetCodeWord()
{
    return CodeWords.IsValidIndex(CodeWord) ? CodeWords[CodeWord] : TEXT("");
}
FString UHotel_Manager::GetWrongCodeWord()
{
   int WrongIndex = 0;

    while (WrongIndex == CodeWord)
    {
        WrongIndex = FMath::RandRange(0, CodeWords.Num() - 1);
    }

    return CodeWords[WrongIndex];
}


void UHotel_Manager::LoadGuestName() //나중에 필요하다면 데이터 에셋 사용
{
    FGuestname temp;
    temp.isMan = false;
    
    temp.Name = TEXT("Lily");         temp.SimilarName = TEXT("Lilia");      arrGuestName.Add(temp);
    temp.Name = TEXT("Grace");        temp.SimilarName = TEXT("Gracy");      arrGuestName.Add(temp);
    temp.Name = TEXT("Sophie");       temp.SimilarName = TEXT("Sophia");     arrGuestName.Add(temp);
    temp.Name = TEXT("Eliza");        temp.SimilarName = TEXT("Elisa");      arrGuestName.Add(temp);
    temp.Name = TEXT("Nora");         temp.SimilarName = TEXT("Norah");      arrGuestName.Add(temp);
    temp.Name = TEXT("Clara");        temp.SimilarName = TEXT("Klara");      arrGuestName.Add(temp);
    temp.Name = TEXT("Eva");          temp.SimilarName = TEXT("Ava");        arrGuestName.Add(temp);
    temp.Name = TEXT("Sara");         temp.SimilarName = TEXT("Sera");       arrGuestName.Add(temp);
    temp.Name = TEXT("Isabelle");     temp.SimilarName = TEXT("IsabeHe");     arrGuestName.Add(temp);
    temp.Name = TEXT("Emily");        temp.SimilarName = TEXT("Emilie");     arrGuestName.Add(temp);
    temp.Name = TEXT("Olivia");       temp.SimilarName = TEXT("Olivya");     arrGuestName.Add(temp);
    temp.Name = TEXT("Hannah");       temp.SimilarName = TEXT("Hanna");      arrGuestName.Add(temp);
    temp.Name = TEXT("Chloe");        temp.SimilarName = TEXT("Khloe");      arrGuestName.Add(temp);
    temp.Name = TEXT("Mia");          temp.SimilarName = TEXT("Mya");        arrGuestName.Add(temp);
    temp.Name = TEXT("Amelia");       temp.SimilarName = TEXT("Emilia");     arrGuestName.Add(temp);
    temp.Name = TEXT("Madeline");     temp.SimilarName = TEXT("Madelyn");    arrGuestName.Add(temp);
    temp.Name = TEXT("Luna");         temp.SimilarName = TEXT("Louna");      arrGuestName.Add(temp);
    temp.Name = TEXT("Zoe");          temp.SimilarName = TEXT("Zoey");       arrGuestName.Add(temp);
    temp.Name = TEXT("Victoria");     temp.SimilarName = TEXT("Viktoria");   arrGuestName.Add(temp);
    temp.Name = TEXT("Natalie");      temp.SimilarName = TEXT("Nathalie");   arrGuestName.Add(temp);

         
    temp.isMan = true;    
    temp.Name = TEXT("Liam");         temp.SimilarName = TEXT("Lian");       arrGuestName.Add(temp);
    temp.Name = TEXT("Ethan");        temp.SimilarName = TEXT("Ethen");      arrGuestName.Add(temp);
    temp.Name = TEXT("Lucas");        temp.SimilarName = TEXT("Luca");       arrGuestName.Add(temp);
    temp.Name = TEXT("Daniel");       temp.SimilarName = TEXT("Danial");     arrGuestName.Add(temp);
    temp.Name = TEXT("Owen");         temp.SimilarName = TEXT("Ewen");       arrGuestName.Add(temp);
    temp.Name = TEXT("Mason");        temp.SimilarName = TEXT("Maison");     arrGuestName.Add(temp);
    temp.Name = TEXT("Julian");       temp.SimilarName = TEXT("Jullian");    arrGuestName.Add(temp);
    temp.Name = TEXT("Isaac");        temp.SimilarName = TEXT("Issac");      arrGuestName.Add(temp);
    temp.Name = TEXT("Aaron");        temp.SimilarName = TEXT("Aaren");      arrGuestName.Add(temp);
    temp.Name = TEXT("Ryan");         temp.SimilarName = TEXT("Rian");       arrGuestName.Add(temp);
    temp.Name = TEXT("Henry");        temp.SimilarName = TEXT("Henri");      arrGuestName.Add(temp);
    temp.Name = TEXT("Noah");         temp.SimilarName = TEXT("Noh");        arrGuestName.Add(temp);
    temp.Name = TEXT("Leo");          temp.SimilarName = TEXT("Leon");       arrGuestName.Add(temp);
    temp.Name = TEXT("Sebastian");    temp.SimilarName = TEXT("Sebastion");  arrGuestName.Add(temp);
    temp.Name = TEXT("Nathan");       temp.SimilarName = TEXT("Nathen");     arrGuestName.Add(temp);
    temp.Name = TEXT("Caleb");        temp.SimilarName = TEXT("Kaleb");      arrGuestName.Add(temp);
    temp.Name = TEXT("Christian");    temp.SimilarName = TEXT("Cristian");   arrGuestName.Add(temp);
    temp.Name = TEXT("Jonathan");     temp.SimilarName = TEXT("Jonathon");   arrGuestName.Add(temp);
    temp.Name = TEXT("Thomas");       temp.SimilarName = TEXT("Tomas");      arrGuestName.Add(temp);
    temp.Name = TEXT("David");        temp.SimilarName = TEXT("Davud");      arrGuestName.Add(temp);

    FString Temp;

    for (auto& GuestName : arrGuestName) //랜덤하게 비슷한 이름 스왑
    {
        if (FMath::RandRange(0, 1))
        {
            Temp = GuestName.Name;
            GuestName.Name = GuestName.SimilarName;
            GuestName.SimilarName = Temp;
        }        
    }
    Algo::RandomShuffle(arrGuestName);
}

void UHotel_Manager::SpawnGuest()
{
    UWorld* World = GetWorld();
    if (!World) return;
    if (arrBPGuestDataAsset.Num() <= 0) return;

    const int32 GuestsToSpawn = FMath::Max(0, MaxGuestsPerRoundHardCap);
    if (GuestsToSpawn <= 0) return;
    
    World->GetTimerManager().ClearTimer(SpawnGuestLoopTimerHandle);
    bStopSpawnOnSettingEvent = false;

    arrWaitingEventList.Empty();

    TWeakObjectPtr<UHotel_Manager> WeakThis(this);

    const FVector SpawnLocation(-1300, 1147, 100);
    const FRotator SpawnRotation = FRotator::ZeroRotator;

    const int32 BatchSize = FMath::Max(1, GuestsPerSpawnTick);
    const float Interval = FMath::Max(0.01f, GuestSpawnTickIntervalSeconds);

    int32 RemainingGuests = GuestsToSpawn;
    SpawnGuestLoopTimerHandle = FTimerHandle();

    World->GetTimerManager().SetTimer(
        SpawnGuestLoopTimerHandle,
        [WeakThis, SpawnLocation, SpawnRotation, BatchSize, RemainingGuests]() mutable
        {
            if (!WeakThis.IsValid()) return;

            UWorld* LocalWorld = WeakThis->GetWorld();
            if (!LocalWorld) return;

            if (WeakThis->bStopSpawnOnSettingEvent)
            {
                LocalWorld->GetTimerManager().ClearTimer(WeakThis->SpawnGuestLoopTimerHandle);
                return;
            }

            if (WeakThis->arrBPGuestDataAsset.Num() <= 0)
            {
                LocalWorld->GetTimerManager().ClearTimer(WeakThis->SpawnGuestLoopTimerHandle);
                return;
            }

            const int32 CanSpawn = FMath::Min(BatchSize, RemainingGuests);
            for (int32 SpawnIdx = 0; SpawnIdx < CanSpawn; ++SpawnIdx)
            {
                const int32 Rand = FMath::RandRange(0, WeakThis->arrBPGuestDataAsset.Num() - 1);
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = nullptr;
                SpawnParams.Instigator = nullptr;

                AHotel_Guest* NewGuest = LocalWorld->SpawnActor<AHotel_Guest>(
                    WeakThis->arrBPGuestDataAsset[Rand]->GuestClass,
                    SpawnLocation,
                    SpawnRotation,
                    SpawnParams);

                if (!NewGuest) continue;

                NewGuest->SetActorHiddenInGame(true);
                NewGuest->SetActorTickEnabled(false);
                NewGuest->SetActorEnableCollision(false);
                if (UCharacterMovementComponent* MoveComp = NewGuest->GetCharacterMovement())
                {
                    MoveComp->GravityScale = 0.0f;
                }
                NewGuest->IsMan = WeakThis->arrBPGuestDataAsset[Rand]->isMan;

                UEventInfo* temp = NewObject<UEventInfo>(WeakThis.Get());
                temp->EventGuest = NewGuest;
                WeakThis->arrWaitingEventList.Add(temp);

                WeakThis->arrEventGuestController.Add(LocalWorld->SpawnActor<AAI_Hotel_Guest_Default>());
                WeakThis->arrHotelGuest.Add(NewGuest);
            }

            RemainingGuests -= CanSpawn;
            if (RemainingGuests <= 0)
            {
                LocalWorld->GetTimerManager().ClearTimer(WeakThis->SpawnGuestLoopTimerHandle);
            }
        },
        Interval,
        true,
        0.0f);
}

void UHotel_Manager::SettingEvent()
{
    if (arrEventFuntionList.IsEmpty()) return;
    if (arrWaitingEventList.IsEmpty()) return;

    // EnterHotel 이후에는 스폰/슬롯 추가가 진행되면 arrWaitingEventList 데이터가 불안정해질 수 있으므로 중단합니다.
    if (UWorld* World = GetWorld())
    {
        bStopSpawnOnSettingEvent = true;
        World->GetTimerManager().ClearTimer(SpawnGuestLoopTimerHandle);
    }

    const int nEventGuestNum = GetEnviromentalLevel() * 4 + 4;

    const int EnviromentLevelLocal = GetEnviromentalLevel();
    if (Hotel_Clock) Hotel_Clock->ChangeEviromentLevel(EnviromentLevelLocal);

    if (EnviromentLevelLocal == 0)
    {
        SettingReservationGuest();
        Algo::RandomShuffle(arrWaitingEventList);
        if (arrWaitingEventList.IsEmpty()) return;
        arrWaitingEventList[0]->isNormalGuestEvent = false;
        UHotel_EventBase* GuestHangingEvent = nullptr;
        for (UHotel_EventBase* Evt : arrEventFuntionList)
        {
            if (Evt && Evt->EventID == EHotelEventId::GuestHanging)
            {
                GuestHangingEvent = Evt;
                break;
            }
        }
        arrWaitingEventList[0]->EventLogic =
            GuestHangingEvent ? GuestHangingEvent : (arrEventFuntionList.IsValidIndex(0) ? arrEventFuntionList[0] : nullptr);

    }
    else
    {
        int nAssignmentEventNum = 0;
        TArray<UHotel_EventBase*> OnceEvents;
        TArray<UHotel_EventBase*> RepeatEvents;
        OnceEvents.Reserve(arrEventFuntionList.Num());
        RepeatEvents.Reserve(arrEventFuntionList.Num());

        for (UHotel_EventBase* Evt : arrEventFuntionList)
        {
            if (!Evt) continue;
            if (Evt->bEventOnlyOnce)
            {
                OnceEvents.Add(Evt);
            }
            else
            {
                RepeatEvents.Add(Evt);
            }
        }

        int32 onceCount = OnceEvents.Num();   // 로컬에서만 소진(선택되면 제거)
        const int32 repeatCount = RepeatEvents.Num(); // repeat은 소진되지 않음

        for (auto& TempEvent : arrWaitingEventList)
        {
            if (nAssignmentEventNum >= nEventGuestNum)
            {
                TempEvent->isNormalGuestEvent = true;
                continue;
            }

            TempEvent->isNormalGuestEvent = false;
            TempEvent->EventLogic = nullptr;

            if (onceCount <= 0 && repeatCount <= 0)
            {
                break;
            }
            const int32 totalCount = onceCount + repeatCount;
            const int32 pick = FMath::RandRange(0, totalCount - 1);
            const bool bPickOnce = (pick < onceCount);

            if (bPickOnce)
            {
                const int32 pickOnceIndex = FMath::RandRange(0, onceCount - 1);
                TempEvent->EventLogic = OnceEvents[pickOnceIndex];
                OnceEvents.Swap(pickOnceIndex, onceCount - 1);
                --onceCount;
            }
            else
            {
                if (repeatCount <= 0)
                {
                    continue;
                }
                const int32 pickRepeatIndex = FMath::RandRange(0, repeatCount - 1);
                TempEvent->EventLogic = RepeatEvents[pickRepeatIndex];
            }

            ++nAssignmentEventNum;
        }
        SettingReservationGuest();
        Algo::RandomShuffle(arrWaitingEventList);

    }


    for (int i = 0; i < arrWaitingEventList.Num(); ++i)
    {
        if (arrWaitingEventList[i] && IsValid(arrWaitingEventList[i]->EventGuest))
        {
            CheckExecuteFunctionTiming(FET_Assignment, arrWaitingEventList[i]->EventGuest);
        }
    }
    if (Hotel_Clock)
    {
        Hotel_Clock->StartGameClock();
    }
    if (Hotel_CCTV && Hotel_CCTV->arrCameras.Num() > 0)
    {
        Hotel_CCTV->arrCameras[0]->SetCameraState(true);
    }
}

void UHotel_Manager::TryCalling(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum)
{
    //CalledPhone 전화를 건 전화기 , NowCallingPhoneNum 전화기가 현재 걸고있는 전화번호
    if (IsGameEndPhase) return;
    FString InstigatorName = TEXT("Walker");
    FString CalledPlaceNumber = TEXT("");
    if (IsValid(CalledPhone))
    {
        CalledPlaceNumber = CalledPhone->RegistPhoneNumber.ToString();
        if (!CalledPhone->isUserPickUpPhone
            && IsValid(CalledPhone->PhoneWatchGuest)
            && !CalledPhone->PhoneWatchGuest->GuestName.IsEmpty())
        {
            InstigatorName = CalledPhone->PhoneWatchGuest->GuestName;
        }
    }

    OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::TryCall,
        {
            { EHotelTriggerKey::Instigator, InstigatorName },
            { EHotelTriggerKey::Place, CalledPlaceNumber },
            { EHotelTriggerKey::Target, NowCallingPhoneNum.ToString() }
        }));

    //전화 예외 처리, 갯수가 적기 때문에 따로 시스템화 X
    if (ProcessCallException(CalledPhone, NowCallingPhoneNum)) return;

    if (!IsValid(CalledPhone)) return;
    if (IsValid(CalledPhone->aConnectedPhone))
    {
        return;
    }
    
    bool bConnectTryStarted = false;
    if (AHotel_Phone** CallingPhonePtr = mapRegistedPhone.Find(NowCallingPhoneNum))
    {
        AHotel_Phone* CallingPhone = *CallingPhonePtr;
        if (IsValid(CallingPhone)
            && !IsValid(CallingPhone->aConnectedPhone)
            && CalledPhone != CallingPhone
            && !CallingPhone->isUserPickUpPhone)
        {
            CallingPhone->ConnectTry(CalledPhone);
            CalledPhone->ConnectTry(CallingPhone);
            bConnectTryStarted = true;
        }
    }
    if (!bConnectTryStarted)
    {
        CalledPhone->ConnectFail();
    }
}

bool UHotel_Manager::ProcessCallException(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum)
{
    static const FName Num205(TEXT("205"));
    static const FName Num305(TEXT("305"));
    static const FName Num911(TEXT("911"));
    const bool bNowIsBlocked = (NowCallingPhoneNum == Num205 || NowCallingPhoneNum == Num305);
    const bool bCalledIsBlocked = (IsValid(CalledPhone)
        && (CalledPhone->RegistPhoneNumber == Num205 || CalledPhone->RegistPhoneNumber == Num305));

    if (bNowIsBlocked || bCalledIsBlocked)
    {
        if (NowCallingPhoneNum == Num205 && IsValid(CalledPhone) && CalledPhone->RegistPhoneNumber == Num205)
        {
            return true; //205호 이벤트를 위해 벽열기 조건시 신호를 연결중 신호 지속
        }

        if (IsValid(CalledPhone))
        {
            CalledPhone->ConnectFail(); //나머지 시도는 연결 끊기
        }
        return true;
    }
    if (NowCallingPhoneNum == Num911)
    {
        if (IsValid(Department_Operator))
        {
            Department_Operator->AddDialogueDataState(TEXT("보안팀"), EDialogueState::DS_Security);
            Department_Operator->SetGuestDialogueDataLast(true);
        }
        return false;
    }
    return false;
}


void UHotel_Manager::MinusHRScore(int MinusScore, FString Reason)
{
    if (IsGameEndPhase)
    {
        return;
    }
    if (HRService)
    {
        HRService->MinusScore(MinusScore, Reason, GetWorld(), Department_Operator, WalkerNowLocation);
        if (HRService->GetScore() <= 0)
        {
            GetWorld()->GetTimerManager().SetTimer(GameEndTimer, [this]()
                {
                    SetGameEnd(GER_Fired);
                }, 10.0f, false);
        }
    }
}
void UHotel_Manager::CheckManagerCallingForHR()
{
    if (HRService)
    {
        HRService->CheckManagerCallingForHR(Department_Operator, WalkerNowLocation);
    }
}

void UHotel_Manager::PlusHRScore(int plusScore, FString Reason)
{
    if (HRService)
    {
        HRService->PlusScore(plusScore, Reason);
    }
}

void UHotel_Manager::AddRegistPhone(FName Number, AHotel_Phone* Phone)
{
    if (Number == "" || !IsValid(Phone)) return;

    mapRegistedPhone.Add(Number, Phone);
}

void UHotel_Manager::AddRegistedPlace(FName Name, AHotel_Place* Place)
{
    if (Name.IsNone() || !IsValid(Place)) return;
    mapRegistedPlace.Add(Name, Place);
}

AHotel_Place* UHotel_Manager::GetPlaceByName(FName Name)
{
    if (mapRegistedPlace.Contains(Name))
    {
        return *mapRegistedPlace.Find(Name);
    }
    return nullptr;
}

void UHotel_Manager::AddRegistedRoom(FName Name, AHotel_Guest_Room* Room)
{
    if (Name.IsNone() || !IsValid(Room)) return;
    FRoomInfo TempInfo;
    TempInfo.Room = Room;
    mapRegistedRoom.Add(Name, TempInfo);
}

void UHotel_Manager::AddHotelMesh(FName Name, UHotel_StaticMesh* Mesh)
{
    if (Name.IsNone() || !IsValid(Mesh)) return;
    mapHotelMesh.Add(Name, Mesh);
}

UHotel_StaticMesh* UHotel_Manager::GetHotelMeshByName(FName Name)
{
    if (mapHotelMesh.Contains(Name))
    {
        return *mapHotelMesh.Find(Name);
    }
    return nullptr;
}

FRoomInfo* UHotel_Manager::FindRoomInfoByName(FName RoomKey)
{
    return mapRegistedRoom.Find(RoomKey);
}


void UHotel_Manager::AddHotelSwitch(FName Name, AHotel_Switch* hotelSwitch)
{
    if (Name.IsNone() || !IsValid(hotelSwitch)) return;
    mapHotelSwitch.Add(Name, hotelSwitch);
}


void UHotel_Manager::UpdateWalkerLocation(FName LocationName)
{
    WalkerNowLocation = LocationName;
    OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::PlaceStateChange,
        {
            { EHotelTriggerKey::Instigator, TEXT("Walker") },
            { EHotelTriggerKey::Place, LocationName.ToString() },
            { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::In) }
        }));

    if (WalkerNowLocation == "Counter")
    {
        if (!arrExecutingEventList.IsEmpty())
        {
            CheckExecuteFunctionTiming(FET_WalkerEnterCounter, arrExecutingEventList.Last()->EventGuest); //카운터로 왔을때 이벤트 실행
        }
        CheckManagerCallingForHR();
    }

    if (mapRegistedRoom.Contains(WalkerNowLocation))
    {
        if (FRoomInfo* Tempmap = mapRegistedRoom.Find(WalkerNowLocation)) //워커 위치가 바뀌었을때 이벤트 실행
        {
            if (Tempmap) //플레이어가 방에 들어왔을때
            {
                if (IsValid(Tempmap->RoomGuest)) //해당방에 게스트가 있을경우
                {
                    AAI_Hotel_Guest_Default* TempAI = Cast<AAI_Hotel_Guest_Default>(Tempmap->RoomGuest->GetController());
                    if (!TempAI->IsMovingToTarget())
                    {
                        Tempmap->RoomGuest->SetLookingPlayer(true); //게스트는 들어온 플레이어를 쳐다 본다
                    }
                }
            }
        }
    }
}

void UHotel_Manager::CheckInGuestRoom(AHotel_Guest* Guest , FName RoomNum)
{
    if (!IsValid(Guest)) return;
    AAI_Hotel_Guest_Default* TempAI = Cast<AAI_Hotel_Guest_Default>(Guest->GetController());
    if(IsValid(TempAI))
    {
        if (mapRegistedRoom.Contains(RoomNum))
        {
            if (FRoomInfo* Tempmap = mapRegistedRoom.Find(RoomNum))
            {
                if (Tempmap && !IsValid(Tempmap->RoomGuest))  //방이 비었다면
                {
                    Tempmap->RoomGuest = Guest;
                    TempAI->AssigningGuestRoom(Tempmap->Room);
                    KeyTray->GiveRoomKey(RoomNum);
                    CheckExecuteFunctionTiming(FET_CheckIn, Guest);
                    if (arrReservationGuest.Find(Guest->GuestName))
                    {
                        arrReservationGuest.Remove(Guest->GuestName);//예약손님 체크인시 명단에서 제외
                    }
                }
            }
        }
    }
}


bool UHotel_Manager::CheckGuestRoom(FName RoomNum)
{
    if (FRoomInfo* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap)
        {
            if (!IsValid(Tempmap->RoomGuest)) 
            {
                return true;//체크인 가능 상태에서만 true
            }
        }
    }
    return false; //나머지 모든 경우에서는 false
}

void UHotel_Manager::CheckOutGuestRoom(FName RoomNum)
{
    if (FRoomInfo* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap)
        {
            Tempmap->Room->CheckOutProcess();
            CheckExecuteFunctionTiming(FET_CheckOut, Tempmap->RoomGuest);
            KeyTray->ReturnRoomKey(RoomNum);
            Tempmap->RoomGuest = NULL;
        }
    }
}

void UHotel_Manager::GuestOutHotel(AHotel_Guest* guest)
{
    for (int i = arrExecutingEventList.Num() - 1; i >= 0; --i)
    {
        if (arrExecutingEventList[i]->EventGuest == guest)
        {
            RemoveExecutingEvent(arrExecutingEventList[i]);
        }
    }
}

void UHotel_Manager::KnockingRoomDoor(FName RoomNum)
{
    if (FRoomInfo* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap)
        {
            if (IsValid(Tempmap->RoomGuest))
            {
                Tempmap->RoomGuest->HearingKnockAction();
            }
        }
    }
}
bool UHotel_Manager::CheckCorrectGuest(FName RoomNum, AHotel_Guest* Guest)
{
    if (FRoomInfo * Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap)
        {
            if (IsValid(Tempmap->RoomGuest) && Guest == Tempmap->RoomGuest)
            {
                return true;
            }
        }
    }
    return false;
}

UDialogueDataAsset* UHotel_Manager::GetDialogueData(FName ConversationName)
{
    if (DialogueMap.Contains(ConversationName))
    {
        return *DialogueMap.Find(ConversationName);
    }
    return NULL;
}

void UHotel_Manager::WalkerCorpseRetrieval()
{
    if (WalkerNowLocation == "205")
    {
        PlusHRScore(20 , TEXT("실종자 소재 파악 및 수습 절차 이행"));
    }
    else
    {
        PlusHRScore(10, TEXT("시체 수습"));
    }
}
void UHotel_Manager::ExcuteEventFail()
{
    MinusHRScore(20, TEXT("EventFail"));
}

void UHotel_Manager::AddNextExecutiongEventList()
{                       
    UE_LOG(LogTemp,Warning,TEXT("넥스트 이벤트 익스 큐팅 : %d"), nNowExcutingEvent);
    if (nNowExcutingEvent > arrWaitingEventList.Num()) return;
    UEventInfo* temp = arrWaitingEventList[nNowExcutingEvent];

    if (!temp->isNormalGuestEvent)
    {
        arrExecutingEventList.Add(temp);
        CheckExecuteFunctionTiming(FET_Init, temp->EventGuest);
        if (WalkerNowLocation == "Counter")
        {
            CheckExecuteFunctionTiming(FET_WalkerEnterCounter, temp->EventGuest);
        }
    }
    else //일반 손님일땐 활성화만
    {
        if (temp->EventGuest)
        {
            ActivateGuest(temp->EventGuest, true);
        }
    }
    nNowExcutingEvent++;
}

void UHotel_Manager::CheckExecuteFunctionTiming(EFunctionExcuteTiming nowTiming, AHotel_Guest* TargetGuest)
{
    for (int i = arrExecutingEventList.Num() - 1; i >= 0; --i)  //삭제로 인덱스 변경 위험 뒤로부터
    {
        if (arrExecutingEventList[i]->isAreadyExcute) continue; //이미 실행된 이벤트면 넘기고
        if (arrExecutingEventList[i]->EventGuest != TargetGuest) continue; //대상 게스트가 이벤트의 게스트가 아니면 넘기고
        UEventInfo* ExecutingEvent = arrExecutingEventList[i];
        if (!ExecutingEvent) continue;

        if (IsValid(ExecutingEvent->EventLogic) && ExecutingEvent->EventLogic->ExecuteTiming == nowTiming)
        {
            ExecutingEvent->EventLogic->Execute(this, ExecutingEvent);
            if (ExecutingEvent->EventLogic->bExecutingOnlyEvent)
            {
                arrExecutingEventList.RemoveAt(i); //진행중 이벤트 목록에서 제거
            }
            //break;  //동일 게스트/동일 타이밍에 매치되는 이벤트있는 경우에도 전부 처리
        }
    }

}

void UHotel_Manager::ActivateGuest(AHotel_Guest* guest, bool goToCounter) //활성화 시키고 카운터로 보낸다
{
    guest->SetActorHiddenInGame(false);
    guest->SetActorEnableCollision(true);
    guest->SetActorTickEnabled(true);
    guest->GetCharacterMovement()->GravityScale = 1.0f;
    
    for (auto & AIcontroller : arrEventGuestController)
    {
        if (AIcontroller->GetPawn() == nullptr)
        {
            AIcontroller->InitAIController(guest);
            break;
        }
    }

    if (goToCounter)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [guest]()
            {
                if (AAI_Hotel_Guest_Default* AI = Cast<AAI_Hotel_Guest_Default>(guest->GetController()))
                {
                    AI->GoToCounter();
                }
            }, 1.0f, false);

        for (auto& temp : mapRegistedRoom)
        {
            if (!IsValid(temp.Value.RoomGuest))
            {
                if (nNowExcutingEvent == 0) AddOutbreakEventList(guest, EHotelOutbreakEventId::MakeDirtyRoom);
                if (nNowExcutingEvent == 3) AddOutbreakEventList(guest, EHotelOutbreakEventId::RequestRejectCheckIn);
                if (nNowExcutingEvent == 6) AddOutbreakEventList(guest, EHotelOutbreakEventId::ImpostorRequestRejectCheckIn);
                if (nNowExcutingEvent == 7) AddOutbreakEventList(guest, EHotelOutbreakEventId::AllocateRoomGuest);

                break;
            }
        }
    }
}

bool UHotel_Manager::OnEventTriggerAction(const FHotelTrigger& Trigger)
{
    bool isTriggeredThisAction = false;
    int TriggerNum = 0;
    if (arrExecutingEventList.Num() > 0)
    {
        for (int i = arrExecutingEventList.Num() - 1; i >= 0; --i)//반복문 진행중 클리어되서 삭제되는 이벤트가 있으므로 역순으로 돌아 인덱스 문제 제거
        {
            TriggerNum = arrExecutingEventList[i]->CollectedTriggers.Num(); //이번 트리거가 추가된건지 확인하기 위해 트리거 작동전 저장
            bool CheckClear = false;
            if (IsValid(arrExecutingEventList[i]->EventLogic))
            {
                CheckClear = arrExecutingEventList[i]->EventLogic->CheckClear(this, arrExecutingEventList[i], Trigger);
            }
            if (!arrExecutingEventList[i]->CollectedTriggers.IsEmpty())
            {
                if (TriggerNum != arrExecutingEventList[i]->CollectedTriggers.Num() && arrExecutingEventList[i]->CollectedTriggers.Last() == Trigger)
                {
                    isTriggeredThisAction = true; //방금 트리거로 콜렉션 갯수가 바뀌었고, 라스트 트리거가 현재 트리거이름이라면 방금 행동으로 트리거 추가된것으로 판정.
                }
            }
            if (CheckClear)
            {
                RemoveExecutingEvent(arrExecutingEventList[i]);
            }
        }
    }
    
    CheckExcuteBasicRule(Trigger);
    return isTriggeredThisAction;
}
void UHotel_Manager::CheckExcuteBasicRule(const FHotelTrigger& Trigger)
{
    const FString* InstigatorPayload = Trigger.Payload.Find(EHotelTriggerKey::Instigator);
    const FString* PlacePayload = Trigger.Payload.Find(EHotelTriggerKey::Place);
    const FString* PlaceState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
    const bool bWalkerEnter = (Trigger.Type == EHotelTriggerType::PlaceStateChange
        && InstigatorPayload && *InstigatorPayload == TEXT("Walker")
        && PlacePayload && PlaceState
        && HotelTriggerStateEquals(PlaceState, EHotelObjectState::In));
    const bool bWalkerOut = (Trigger.Type == EHotelTriggerType::PlaceStateChange
        && InstigatorPayload && *InstigatorPayload == TEXT("Walker")
        && PlacePayload && PlaceState
        && HotelTriggerStateEquals(PlaceState, EHotelObjectState::Out));

    for (auto& temp : mapRegistedRoom)
    {
        if (IsValid(temp.Value.RoomGuest)) //이미 체크인된 방
        {
            if (temp.Value.RoomGuest->IsHanging) return;//목이 매달린상태면 리턴
            const FString* DoorRoom = Trigger.Payload.Find(EHotelTriggerKey::Place);
            const FString* DoorState = Trigger.Payload.Find(EHotelTriggerKey::ObjectState);
            if (Trigger.Type == EHotelTriggerType::DoorStateChange
                && DoorRoom && DoorState
                && *DoorRoom == temp.Value.Room->RoomNumber.ToString()
                && HotelTriggerStateEquals(DoorState, EHotelObjectState::Open))
            {
                temp.Value.RoomGuest->GetAIController()->CloseRoomDoor(10.0f); // 문만 열려있음 10초뒤에 닫으러 간다
                temp.Value.RoomGuest->AddDialogueDataState(TEXT("무단 침입에 관하여"), EDialogueState::DS_Guest_TresPass, 0);
                temp.Value.RoomGuest->SetGuestDialogueDataLast(false);
                temp.Value.RoomGuest->OpenConversationUI();
                temp.Value.RoomGuest->SetLookingPlayer(true);
            }

            if (bWalkerEnter && *PlacePayload == temp.Value.Room->RoomNumber.ToString()) //플레이어가 방에 들어 갔다면 AI타이머 멈춘다.
            {
                temp.Value.RoomGuest->GetAIController()->StopAITimer();
            }

            if (bWalkerOut && *PlacePayload == temp.Value.Room->RoomNumber.ToString()) //플레이어가 방나간뒤 문닫기 시작
            {
                temp.Value.RoomGuest->GetAIController()->CloseRoomDoor(5.0f);
            }
        }
    }
}

void UHotel_Manager::AddOutbreakEventList(AHotel_Guest* guest, EHotelOutbreakEventId eventId)
{
    if (eventId == EHotelOutbreakEventId::None) return;

    UHotel_OutbreakEventBase* Outbreak = nullptr;
    for (UHotel_OutbreakEventBase* Evt : OutbreakEventObjects)
    {
        if (Evt && Evt->OutbreakId == eventId)
        {
            Outbreak = Evt;
            break;
        }
    }

    if (!Outbreak) return;

    UEventInfo* temp = NewObject<UEventInfo>();
    temp->EventGuest = guest;
    temp->EventLogic = Outbreak;
    temp->isNormalGuestEvent = false;
    arrOutbreakEventList.Add(temp);
    arrExecutingEventList.Add(temp);

    if (!arrExecutingEventList.IsEmpty())
    {
        CheckExecuteFunctionTiming(FET_Init, guest);
        if (WalkerNowLocation == "Counter")
        {
            CheckExecuteFunctionTiming(FET_WalkerEnterCounter, guest);
        }
    }
}

void UHotel_Manager::RemoveExecutingEvent(UEventInfo* TartgetEvent)
{
    GetWorld()->GetTimerManager().ClearTimer(TartgetEvent->EventTimer);
}

void UHotel_Manager::SettingReservationGuest()
{
    if (arrWaitingEventList.IsEmpty()) return;
    int MaxReservationNum = FMath::RandRange(0, 1);
    for (auto& eventInfo : arrWaitingEventList)
    {
        if (eventInfo && IsValid(eventInfo->EventGuest))
        {
            eventInfo->EventGuest->IsReservationGuest = false;
        }
    }

    TArray<int> RandArr;
    RandArr.Reserve(arrWaitingEventList.Num());
    for (int i = 0 ; i < arrWaitingEventList.Num(); i++)
    {
        RandArr.Add(i);
    }
    Algo::RandomShuffle(RandArr);

    for (int i = 0 ; i < MaxReservationNum; i++)
    {
        int32 Index = RandArr[i];
        if (!arrWaitingEventList.IsValidIndex(Index) || !IsValid(arrWaitingEventList[Index]->EventGuest)) continue;

        AHotel_Guest* Guest = arrWaitingEventList[Index]->EventGuest;
        FGuestname tempGuestName = GetGenderName(Guest->IsMan);

        Guest->SetGuestName(tempGuestName.Name);
        arrReservationGuest.Add(tempGuestName.Name);
        Guest->IsReservationGuest = true;

        if (i + 1 < RandArr.Num() && FMath::RandRange(0, 3) < 1)
        {
            int32 SimilarIndex = RandArr[++i];
            if (!arrWaitingEventList.IsValidIndex(SimilarIndex) || !IsValid(arrWaitingEventList[SimilarIndex]->EventGuest)) continue;

            AHotel_Guest* SimilarGuest = arrWaitingEventList[SimilarIndex]->EventGuest;
            SimilarGuest->SetGuestName(tempGuestName.SimilarName);
            SimilarGuest->IsReservationGuest = true;
        }
    }

    for (const auto& eventInfo : arrWaitingEventList)
    {
        if (!eventInfo || !IsValid(eventInfo->EventGuest)) continue;

        if (eventInfo->EventGuest->GuestName.IsEmpty())
        {
            FGuestname tempGuestName = GetGenderName(eventInfo->EventGuest->IsMan);
            eventInfo->EventGuest->SetGuestName(tempGuestName.Name);
        }
    }
}

FString UHotel_Manager::GetAllocateRoomNum()
{
    for (auto& Elem :arrExecutingEventList)
    {
        if (IsValid(Elem->EventGuest))
        {
            if (Elem->CollectedTriggers.Num() > 0)
            {
                const FHotelTrigger& LastTrigger = Elem->CollectedTriggers.Last();
                const FString AllocatedRoom = LastTrigger.Payload.FindRef(EHotelTriggerKey::Target);
                if (LastTrigger.Type == EHotelTriggerType::RoomAssigned && !AllocatedRoom.IsEmpty())
                {
                    if (Elem->isAreadyExcute == false)
                    {
                        Elem->isAreadyExcute = true;
                        return AllocatedRoom;
                    }
                }
            }
        }
    }
    return "";
}

void UHotel_Manager::SaveLevelOption(int menual, int Enviroment)
{
    if (PersistenceService)
    {
        PersistenceService->SaveLevelOption(menual, Enviroment);
    }
}

void UHotel_Manager::SaveGameLevelOption()
{
    if (PersistenceService)
    {
        PersistenceService->SaveGameLevelOption();
    }
}

void UHotel_Manager::LoadGameLevelOption()
{
    if (PersistenceService)
    {
        PersistenceService->LoadGameLevelOption();
    }
}

bool UHotel_Manager::LoadGameManualExternal()
{
    if (PersistenceService)
    {
        return PersistenceService->LoadGameManualExternal();
    }
    return false;
}

bool UHotel_Manager::SaveGameManualExternal()
{
    if (PersistenceService)
    {
        return PersistenceService->SaveGameManualExternal();
    }
    return false;
}

void UHotel_Manager::InitMenualInfo()
{
    if (PersistenceService)
    {
        PersistenceService->InitMenualInfo();
    }
}

void UHotel_Manager::UpdateDefualtLevelMenual(int EventId)
{
    if (PersistenceService)
    {
        PersistenceService->UpdateDefualtLevelMenual(EventId);
    }
}

int UHotel_Manager::GetEnviromentalLevel()
{
    return PersistenceService ? PersistenceService->GetEnviromentalLevel() : 1;
}

int UHotel_Manager::GetMenualLevel()
{
    return PersistenceService ? PersistenceService->GetMenualLevel() : 1;
}

void UHotel_Manager::SetFioneerMenaulText(FString temp)
{
    if (PersistenceService)
    {
        PersistenceService->SetFioneerMenaulText(temp);
    }
}

FString UHotel_Manager::GetFioneerMenaulText()
{
    return PersistenceService ? PersistenceService->GetFioneerMenaulText() : FString();
}

TArray<FManualInfo>* UHotel_Manager::GetMenualInfo()
{
    return PersistenceService ? PersistenceService->GetMenualInfo() : nullptr;
}

TArray<int>* UHotel_Manager::GetExperiencedEventID()
{
    return PersistenceService ? PersistenceService->GetExperiencedEventID() : nullptr;
}

TArray<FHRRecord> UHotel_Manager::GetHRRecord()
{
    return HRService ? HRService->GetRecords() : TArray<FHRRecord>();
}

int UHotel_Manager::GetHRScore()
{
    return HRService ? HRService->GetScore() : 0;
}

bool UHotel_Manager::GetWalkerFired()
{
    return HRService ? HRService->GetWalkerFired() : false;
}

void UHotel_Manager::SetGameEnd(EGameEndReason Reason)
{
    if (IsGameEndPhase) return;
    IsGameEndPhase = true;
    OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::GameEnd));

    for (int i = nNowExcutingEvent; i < arrWaitingEventList.Num(); i++)
    {
        if (arrWaitingEventList[i] && IsValid(arrWaitingEventList[i]->EventGuest))
        {
            arrReservationGuest.Remove(arrWaitingEventList[i]->EventGuest->GuestName);
        }
    } //게임 종료시 호텔 들어오기전 예약손님은 노쇼로 감점사항 X

    for (auto & temp : arrReservationGuest)
    {
        MinusHRScore(10, TEXT("예약손님 입실 거부"));
    }

    EndReason = Reason;
    GetWorld()->GetTimerManager().ClearTimer(GameEndTimer);

    if (Hotel_Walker)
    {
        Hotel_Walker->HideUIName("InteractBox");
        Hotel_Walker->HideUIName("InteractMessage");
        Hotel_Walker->HideUIName("Menual");
        Hotel_Walker->HideUIName("Dialogue");
        Hotel_Walker->HideUIName("Phone");
        Hotel_Walker->SetActorTickEnabled(false);
    }
    for (auto& temp : arrEventGuestController)
    {
        if (IsValid(temp->Hotel_Guest))
        {
            temp->Hotel_Guest->TeleportTo(FVector(-327, 839, 94), FRotator(0, 0, 0));
            temp->Hotel_Guest->DeactivateGuest();
        }
        temp->StopAITimer();
        temp->StopPatienceTimer();
    }

    if (Level_Manager)
    {
        if (Reason == GER_Fired)
        {
            Hotel_Clock->StopGameClock();
        }
        Level_Manager->HandleGameEnd(Reason);
    }

}

void UHotel_Manager::SetMainMenu()
{
    if (Level_Manager)
    {
        Level_Manager->ShowMainMenu();
    }
}
void UHotel_Manager::SettingInitGame(bool isContinue)
{
    arrGuestName.Empty();
    mapRegistedPhone.Empty();
    mapRegistedPlace.Empty();
    mapRegistedRoom.Empty();
    mapHotelMesh.Empty();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(GameEndTimer);
        World->GetTimerManager().ClearTimer(SpawnGuestLoopTimerHandle);
        for (auto& temp : arrExecutingEventList)
        {
            if (temp)
            {
                World->GetTimerManager().ClearTimer(temp->EventTimer);
            }
        }
    }

    arrExecutingEventList.Empty();
    arrOutbreakEventList.Empty();
    arrWaitingEventList.Empty();

    for (auto* Ctrl : arrEventGuestController)
    {
        if (IsValid(Ctrl))
        {
            Ctrl->Destroy();
        }
    }
    arrEventGuestController.Empty();

    for (auto* Guest : arrHotelGuest)
    {
        if (IsValid(Guest))
        {
            Guest->Destroy();
        }
    }
    arrHotelGuest.Empty();

    arrReservationGuest.Empty();
    if (HRService)
    {
        HRService->ResetForNewGame();
    }

    IsContinueSetting = isContinue;
    IsGameEndPhase = false;
    EndReason = GER_NotYet;
    nNowExcutingEvent = 0;


    CodeWord = FMath::RandRange(0, 9);
    LoadGuestName();

}

void UHotel_Manager::CompleteMainLevelLoad()
{
    if (IsContinueSetting)
    {
        IsContinueSetting = false;
        nWalkingDay++;
        Level_Manager->MainMenuUI->EnterButtonAction();
    }
    else
    {
        if (HRService)
        {
            HRService->ResetForNewGame();
        }
        nWalkingDay = 1;
    }
}

FString UHotel_Manager::GetReservationGuestName()
{
    FString TempString;
    for (int i = 0 ; i < arrReservationGuest.Num() ; i++)
    {
        if(i != 0) TempString += ", ";
        TempString += arrReservationGuest[i];
    }
    return TempString;
}

void UHotel_Manager::RingingBell()
{
    Level_Manager->RingingBell();
}
