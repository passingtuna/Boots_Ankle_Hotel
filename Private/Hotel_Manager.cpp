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
        // 이벤트 클래스 등록 (기존 시스템은 유지하고, 내부 로직만 클래스로 분리)
        UHotel_Event_Invader* Invader = NewObject<UHotel_Event_Invader>(this);
        EventObjects.Add(Invader);

        UHotel_Event_StareUnderLight* Stare = NewObject<UHotel_Event_StareUnderLight>(this);
        EventObjects.Add(Stare);

        EventObjects.Add(NewObject<UHotel_Event_GuestRoomCCTV>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestHanging>(this));
        EventObjects.Add(NewObject<UHotel_Event_Open205>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestLostSignalCCTV>(this));
        EventObjects.Add(NewObject<UHotel_Event_GuestInvisibleCamera>(this));
    }

    if (EventById.IsEmpty() && !EventObjects.IsEmpty())
    {
        for (UHotel_EventBase* Evt : EventObjects)
        {
            if (!Evt) continue;
            if (Evt->EventID != EHotelEventId::None)
            {
                EventById.Add(Evt->EventID, Evt);
            }
        }
    }

    if (OutbreakEventObjects.IsEmpty())
    {
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_ComplainRoomDirty>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_RequestRejectCheckIn>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_AllocateRoomGuest>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_MakeDirtyRoom>(this));
        OutbreakEventObjects.Add(NewObject<UHotel_Outbreak_ImpostorRequestRejectCheckIn>(this));
    }

    if (OutbreakById.IsEmpty() && !OutbreakEventObjects.IsEmpty())
    {
        for (UHotel_OutbreakEventBase* Evt : OutbreakEventObjects)
        {
            if (!Evt) continue;
            if (Evt->OutbreakId != EHotelOutbreakEventId::None)
            {
                OutbreakById.Add(Evt->OutbreakId, Evt);
            }
        }
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

    FExcuteFunctionInfo tempEFI;
    tempEFI.EventID = 1;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::Invader))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::Invader))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);
    
    tempEFI.EventID = 3;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestRoomCCTV))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestRoomCCTV))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 5;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestHanging))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestHanging))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 6;    
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::Open205))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::Open205))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = true;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);
    
    tempEFI.EventID = 7;    
    tempEFI.ExecuteTiming = FET_WalkerEnterCounter;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::StareUnderLight))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::StareUnderLight))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 8;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestLostSignalCCTV))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestLostSignalCCTV))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 9;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestInvisibleCamera))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_EventBase* Evt = EventById.FindRef(EHotelEventId::GuestInvisibleCamera))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    ////////////랜덤 실행 이벤트 정의

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::ComplainRoomDirty))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::ComplainRoomDirty))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    mapOutbreakEventFuntion.Add(EHotelOutbreakEventId::ComplainRoomDirty, tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::RequestRejectCheckIn))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::RequestRejectCheckIn))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    mapOutbreakEventFuntion.Add(EHotelOutbreakEventId::RequestRejectCheckIn, tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::AllocateRoomGuest))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.CheckClearFunction = [this](UEventInfo* eventInfo, FName trigger)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::AllocateRoomGuest))
            {
                return Evt->CheckClear(this, eventInfo, trigger);
            }
            return false;
        };
    tempEFI.isEventOnlyOnce = false;
    mapOutbreakEventFuntion.Add(EHotelOutbreakEventId::AllocateRoomGuest, tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::MakeDirtyRoom))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = true;
    mapOutbreakEventFuntion.Add(EHotelOutbreakEventId::MakeDirtyRoom, tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](UEventInfo* eventInfo)
        {
            if (UHotel_OutbreakEventBase* Evt = OutbreakById.FindRef(EHotelOutbreakEventId::ImpostorRequestRejectCheckIn))
            {
                Evt->Execute(this, eventInfo);
            }
        };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = true;
    mapOutbreakEventFuntion.Add(EHotelOutbreakEventId::ImpostorRequestRejectCheckIn, tempEFI);
    SettingInitGame(false);
}
FGuestname UHotel_Manager::GetGenderName(bool isMan)
{
    FGuestname tempGuestName;
    tempGuestName.Name = "";
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
    for (int i = 0; i < 25; ++i)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
            {
                FVector SpawnLocation = FVector(-1300, 1147, 100); // 위치를 얻는 함수
                FRotator SpawnRotation = FRotator::ZeroRotator;
                int Rand = FMath::RandRange(0, arrBPGuestDataAsset.Num() - 1);
                FActorSpawnParameters SpawnParams;
                SpawnParams.Owner = nullptr; // 필요시 세팅
                SpawnParams.Instigator = nullptr;
                AHotel_Guest* NewGuest = GetWorld()->SpawnActor<AHotel_Guest>(arrBPGuestDataAsset[Rand]->GuestClass, SpawnLocation, SpawnRotation, SpawnParams);
                if (NewGuest)
                {
                    NewGuest->SetActorHiddenInGame(true);
                    NewGuest->SetActorTickEnabled(false);
                    NewGuest->SetActorEnableCollision(false);
                    NewGuest->GetCharacterMovement()->GravityScale = 0.0f;//일단 중력을 끄고 액티브할때 켜중;
                    NewGuest->IsMan = arrBPGuestDataAsset[Rand]->isMan;
                    UEventInfo* temp = NewObject<UEventInfo>();
                    temp->EventGuest = NewGuest;
                    arrWaitingEventList.Add(temp);
                    arrEventGuestController.Add(GetWorld()->SpawnActor<AAI_Hotel_Guest_Default>());
                    arrHotelGuest.Add(NewGuest);
                }
            }, 0.1 * i, false);
    }
}
void UHotel_Manager::SettingEvent()
{
    int nRandRange = arrEventFuntionList.Num() - 1;
    int nEventGuestNum;

    const int EnviromentLevelLocal = GetEnviromentalLevel();
    nEventGuestNum = nEventGuestNum = EnviromentLevelLocal * 4 + 4;
    if (Hotel_Clock) Hotel_Clock->ChangeEviromentLevel(EnviromentLevelLocal);

    if (EnviromentLevelLocal == 0)
    {
        SettingReservationGuest(); //예약자 이름까지 설정후
        Algo::RandomShuffle(arrWaitingEventList); // 랜덤으로 섞기
        UE_LOG(LogTemp,Warning,TEXT("테스트용 세팅"));
        arrWaitingEventList[0]->isNormalGuestEvent = false;
        arrWaitingEventList[0]->FunctionInfo = arrEventFuntionList[3];

    }
    else
    {
        // UE_LOG(LogTemp, Warning, TEXT("세팅 이벤트 넘 : %d"), EnviromentLevel);
        int nAssignmentEventNum = 0;
        for (auto& TempEvent : arrWaitingEventList)
        {
            if (nAssignmentEventNum < nEventGuestNum)   //
            {
                nAssignmentEventNum++;
                TempEvent->isNormalGuestEvent = false;
                int randNum = FMath::RandRange(0, nRandRange);
                TempEvent->FunctionInfo = arrEventFuntionList[randNum];
                if (TempEvent->FunctionInfo.isEventOnlyOnce) //게임내 1번만 할당해야될 함수라면
                {
                    arrEventFuntionList.Swap(randNum, nRandRange); //현재 랜덤 뽑는 범위의 제일뒤에 함수와 스왑
                    nRandRange--;//랜덤 값 범위를 1개 줄여 스왑 함수를 랜덤 배치에서 제외
                }
            }
            else
            {
                TempEvent->isNormalGuestEvent = true; //일반 손님
            }
        }
        SettingReservationGuest(); //예약자 이름까지 설정후
        Algo::RandomShuffle(arrWaitingEventList); // 랜덤으로 섞기

    }


    for (int i = 0; i < arrWaitingEventList.Num(); ++i)
    {
        CheckExecuteFunctionTiming(FET_Assignment, arrWaitingEventList[i]->EventGuest);
    }
    Hotel_Clock->StartGameClock();
    Hotel_CCTV->arrCameras[0]->SetCameraState(true);
}

void UHotel_Manager::TryCalling(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum)
{
    //CalledPhone 전화를 건 전화기 , NowCallingPhoneNum 전화기가 현재 걸고있는 전화번호
    if (IsGameEndPhase) return;
    FString tempTriggerStr = "WalkerTryCall_" + CalledPhone->RegistPhoneNumber.ToString() + "_To_" + NowCallingPhoneNum.ToString();
    OnEventTriggerAction(FName(tempTriggerStr));

    //전화 예외 처리, 갯수가 적기 때문에 따로 시스템화 X
    if (ProcessCallException(CalledPhone, NowCallingPhoneNum)) return;
    
    if (mapRegistedPhone.Contains(NowCallingPhoneNum))
    {
        AHotel_Phone* CallingPhone = *mapRegistedPhone.Find(NowCallingPhoneNum);
        if (IsValid(CallingPhone) && !IsValid(CallingPhone->aConnectedPhone) && CalledPhone != CallingPhone && !CallingPhone->isUserPickUpPhone)
        {
            CallingPhone->ConnectTry(CalledPhone);
            CalledPhone->ConnectTry(CallingPhone);
        }
        else
        {
            CalledPhone->ConnectFail();
        }
    }
    else
    {
        CalledPhone->ConnectFail();
    }
}

bool UHotel_Manager::ProcessCallException(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum)
{
    //205,305호는 등록되어있지만 통신 차단 필요
    if (NowCallingPhoneNum == "205" || NowCallingPhoneNum == "305" || CalledPhone->RegistPhoneNumber == "205" || CalledPhone->RegistPhoneNumber == "305")
    {
        if (NowCallingPhoneNum == "205" && CalledPhone->RegistPhoneNumber == "205")
        {
            return true; //205호 이벤트를 위해 벽열기 조건시 신호를 연결중 신호 지속
        }
        CalledPhone->ConnectFail(); //나머지 시도는 연결 끊기
        return true;
    }
    else if (NowCallingPhoneNum == "911") //보안팀 역할이 커진다면 Security_Operator분리 후 해당 조건문 삭제
    {
        //Department_Operator는 기본적으로 호텔 매니저 역할 보안팀 전화일때만 역할 바꾸는걸로 구현
        Department_Operator->AddDialogueDataState("보안팀", EDialogueState::DS_Security);
        Department_Operator->SetGuestDialogueDataLast(true);
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
    FString tempString = "Walker_Enter_" + LocationName.ToString();
    OnEventTriggerAction(FName(tempString));

    UE_LOG(LogTemp, Warning, TEXT("업데이트 로케이션 %s"), *LocationName.ToString());
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
        if (arrExecutingEventList[i]->FunctionInfo.ExecuteTiming == nowTiming)
        {
            arrExecutingEventList[i]->FunctionInfo.ExecuteFunction(arrExecutingEventList[i]);
            if (arrExecutingEventList[i]->FunctionInfo.isExcutingOnlyEvent)
            {
                arrExecutingEventList.RemoveAt(i); //진행중 이벤트 목록에서 제거
            }
            break;//실행했으면 브레이크
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

                 /*
                int rand = FMath::RandRange(1, 100);
                if (rand < 10)
                {
                    AddOutbreakEventList(guest, EHotelOutbreakEventId::AllocateRoomGuest);
                }
                else if (rand < 20)
                {
                    if (!arrReservationGuest.Find(guest->GuestName))
                    {
                        //AddOutbreakEventList(guest, EHotelOutbreakEventId::RequestRejectCheckIn);
                    }
                }*/
                    break;
            }
        }
    }
}

bool UHotel_Manager::OnEventTriggerAction(FName triggerName)
{
    bool isTriggeredThisAction = false;
    int TriggerNum = 0;
    if (arrExecutingEventList.Num() > 0)
    {
        for (int i = arrExecutingEventList.Num() - 1; i >= 0; --i)//반복문 진행중 클리어되서 삭제되는 이벤트가 있으므로 역순으로 돌아 인덱스 문제 제거
        {
            TriggerNum = arrExecutingEventList[i]->CollectedTriggers.Num(); //이번 트리거가 추가된건지 확인하기 위해 트리거 작동전 저장
            bool CheckClear = arrExecutingEventList[i]->FunctionInfo.CheckClearFunction(arrExecutingEventList[i], triggerName);
            if (!arrExecutingEventList[i]->CollectedTriggers.IsEmpty())
            {
                if (TriggerNum != arrExecutingEventList[i]->CollectedTriggers.Num() && arrExecutingEventList[i]->CollectedTriggers.Last() == triggerName)
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
    
    CheckExcuteBasicRule(triggerName);
    return isTriggeredThisAction;
}
void UHotel_Manager::CheckExcuteBasicRule(FName TriggerName)
{
    FString tempTrigeer =TriggerName.ToString();

    for (auto& temp : mapRegistedRoom)
    {
        if (IsValid(temp.Value.RoomGuest)) //이미 체크인된 방
        {
            if (temp.Value.RoomGuest->IsHanging) return;//목이 매달린상태면 리턴
            FString tempRoomTrigger = temp.Value.Room->RoomNumber.ToString() +"Door_Open";
            if (FName(tempRoomTrigger) == TriggerName)
            {
                temp.Value.RoomGuest->GetAIController()->CloseRoomDoor(10.0f); // 문만 열려있음 10초뒤에 닫으러 간다
                temp.Value.RoomGuest->AddDialogueDataState(TEXT("무단 침입에 관하여"), EDialogueState::DS_Guest_TresPass, 0);
                temp.Value.RoomGuest->SetGuestDialogueDataLast(false);
                temp.Value.RoomGuest->OpenConversationUI();
                temp.Value.RoomGuest->SetLookingPlayer(true);
            }

            tempRoomTrigger = "Walker_Enter_" + temp.Value.Room->RoomNumber.ToString();
            if (FName(tempRoomTrigger) == TriggerName) //플레이어가 방에 들어 갔다면 AI타이머 멈춘다.
            {
                temp.Value.RoomGuest->GetAIController()->StopAITimer();
            }

            tempRoomTrigger = "Walker_Out_" + temp.Value.Room->RoomNumber.ToString();
            if (FName(tempRoomTrigger) == TriggerName) //플레이어가 방나간뒤 문닫기 시작
            {
                temp.Value.RoomGuest->GetAIController()->CloseRoomDoor(5.0f);
            }
        }
    }
}

void UHotel_Manager::AddOutbreakEventList(AHotel_Guest* guest, EHotelOutbreakEventId eventId)
{
    if (eventId == EHotelOutbreakEventId::None) return;

    if (mapOutbreakEventFuntion.Contains(eventId))
    {
        const FExcuteFunctionInfo FoundEvent = *mapOutbreakEventFuntion.Find(eventId);
        UEventInfo* temp = NewObject<UEventInfo>();
        temp->EventGuest = guest;
        temp->FunctionInfo = FoundEvent;
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
}

void UHotel_Manager::RemoveExecutingEvent(UEventInfo* TartgetEvent)
{
    GetWorld()->GetTimerManager().ClearTimer(TartgetEvent->EventTimer); //타이머 정지
    //if (arrExecutingEventList.Contains(TartgetEvent))
    //{
    //    int tempIndex = arrExecutingEventList.IndexOfByKey(TartgetEvent);
    //    arrExecutingEventList.RemoveAt(tempIndex); //진행중 이벤트 목록에서 제거
    //}
}

void UHotel_Manager::SettingReservationGuest()
{
    int MaxReservationNum = FMath::RandRange(0, 1); //예약손님은 최대 4명까지 랜덤으로 설정 총 이벤트 갯수는 6개 보단 무조건 많을 예정
    
    FString TempSimliarName;
      
    TArray<int> RandArr;

    for (int i = 0 ;  i < arrWaitingEventList.Num(); i++)
    {
        RandArr.Add(i); //현재 이벤트 갯수만큼 숫자 추가
    }
    Algo::RandomShuffle(RandArr); // 셔플

    for (int i = 0 ; i < MaxReservationNum; i++)
    {
        int32 Index = RandArr[i];
        FGuestname tempGuestName = GetGenderName(arrWaitingEventList[Index]->EventGuest->IsMan);
        arrWaitingEventList[Index]->EventGuest->SetGuestName(tempGuestName.Name);
        arrReservationGuest.Add(tempGuestName.Name); //예약손님 리스트에 추가
        arrWaitingEventList[Index]->EventGuest->IsReservationGuest = true;
        
        if (i +1 < RandArr.Num() && FMath::RandRange(0, 3) < 1) // 1/4 확률로 예약자와 비슷한 이름 할당 성별에 상관없이
        {
            int32 SimilarIndex = RandArr[++i];
            arrWaitingEventList[SimilarIndex]->EventGuest->SetGuestName(tempGuestName.SimilarName);
            arrWaitingEventList[SimilarIndex]->EventGuest->IsReservationGuest = true;
        }
    }
    for (const auto& eventInfo : arrWaitingEventList)
    {
        if (eventInfo->EventGuest->GuestName.IsEmpty()) // 예약관련된 손님이 아니라면 랜덤 할당
        {
            FGuestname tempGuestName = GetGenderName(eventInfo->EventGuest->IsMan);
            eventInfo->EventGuest->SetGuestName(tempGuestName.Name);
        }
    } 
}

FString UHotel_Manager::GetAllocateRoomNum()
{
    TArray<FString> Result;
    for (auto& Elem :arrExecutingEventList)
    {
        if (IsValid(Elem->EventGuest))
        {
            if (Elem->CollectedTriggers.Num() > 0)
            {
                FString tempString = Elem->CollectedTriggers.Last().ToString();
                tempString.ParseIntoArray(Result,TEXT("_"));
                if (Result[0] == "AllocateRoomNum")
                {
                    if (Elem->isAreadyExcute == false)
                    {
                        Elem->isAreadyExcute = true;
                        return Result[1];
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
    OnEventTriggerAction("GameEnd");//해결못하고 엔드할시 패널티 적용

    for (int i = nNowExcutingEvent; i < arrWaitingEventList.Num(); i++)
    {
        arrReservationGuest.Remove(arrWaitingEventList[i]->EventGuest->GuestName);
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
    arrEventGuestController.Empty();
    arrReservationGuest.Empty();
    arrWaitingEventList.Empty();
    if (HRService)
    {
        HRService->ResetForNewGame();
    }

    for (auto & temp : arrExecutingEventList)
    {
        GetWorld()->GetTimerManager().ClearTimer(temp->EventTimer);
    }
    arrExecutingEventList.Empty();

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
     
    UE_LOG(LogTemp, Warning, TEXT("리절베이션 게스트 %s") , *TempString);
    return TempString;
}

void UHotel_Manager::RingingBell()
{
    Level_Manager->RingingBell();
}
