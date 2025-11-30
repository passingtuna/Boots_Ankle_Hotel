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


void UHotel_Manager::Initialize(FSubsystemCollectionBase& Collection)
{
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

    MenualSaveName = TEXT("HotelManual.txt");          // 파일 이름

    UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/BluePrint/Data/Table/DT_Menual"));
    DefualtMenualText.Empty();
    if (Table)
    {
        static const FString Context(TEXT("DT_Menual"));
        TArray<FManualInfo*> TempRow;
        Table->GetAllRows<FManualInfo>(Context, TempRow);
        for (auto& Row : TempRow)
        {
            DefualtMenualText.Add(*Row);
           // UE_LOG(LogTemp, Warning, TEXT("Menual Text : %s"), *Row->MenualText.ToString());
        }
    }


    LoadGameLevelOption();
    if (!LoadGameManualExternal())
    {
        //로드 실패시 초기화
        InitMenualInfo();
    }

    FExcuteFunctionInfo tempEFI;
    tempEFI.EventID = 1;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Event_Invader(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Event_Invader(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);
    
    tempEFI.EventID = 3;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {ExecuteEvent_Guest_RoomCCTV(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Guest_RoomCCTV(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 5;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {ExecuteEvent_Guest_Hanging(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Guest_Hanging(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 6;    
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Event_Open205(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo , FName trigger) {return CheckClear_Event_Open205(eventInfo,trigger); };
    tempEFI.isEventOnlyOnce = true;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);
    
    tempEFI.EventID = 7;    
    tempEFI.ExecuteTiming = FET_WalkerEnterCounter;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Event_StarePeopleUnderLight(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Event_StarePeopleUnderLight(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);

    tempEFI.EventID = 8;
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {ExecuteEvent_Guest_LostSignalCCTV(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Guest_LostSignalCCTV(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = false;
    arrEventFuntionList.Add(tempEFI);


    ////////////랜덤 실행 이벤트 정의

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Event_Complain_RoomDirty(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Complain_RoomDirty(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    mapOutbreakEventFuntion.Add(FName("ComplainRoomDirty"), tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Event_Request_Reject_CheckIn(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Request_Reject_CheckIn(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    mapOutbreakEventFuntion.Add(FName("RequestRejectCheckIn"), tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Event_Allocate_Room(eventInfo); };
    tempEFI.CheckClearFunction = [this](FEventInfo* eventInfo, FName trigger) { return CheckClear_Allocate_Room(eventInfo, trigger); };
    tempEFI.isEventOnlyOnce = false;
    mapOutbreakEventFuntion.Add(FName("Allocate_RoomGuest"), tempEFI);

    tempEFI.EventID = 0; //따로 메뉴얼에 표시되는 이벤트 아님
    tempEFI.ExecuteTiming = FET_Init;
    tempEFI.ExecuteFunction = [this](FEventInfo* eventInfo) {Execute_Only_MakeDirtyRoom(eventInfo); };
    tempEFI.isEventOnlyOnce = false;
    tempEFI.isExcutingOnlyEvent = true;
    mapOutbreakEventFuntion.Add(FName("MakeDirtyRoom"), tempEFI);
    
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
    switch (CodeWord)
    {
        case 0: return TEXT("앞코"); break;
        case 1: return TEXT("뒷굽"); break;
        case 2: return TEXT("부츠끈"); break;
        case 3: return TEXT("혀"); break;
        case 4: return TEXT("목통"); break;
        case 5: return TEXT("밑창"); break;
        case 6: return TEXT("덧창"); break;
        case 7: return TEXT("깔창"); break;
        case 8: return TEXT("안창"); break;
        case 9: return TEXT("장식못"); break;
        default: return ""; break;
    }
}
FString UHotel_Manager::GetWrongCodeWord()
{
    int WrongCodeWord = FMath::RandRange(0, 8); //잘못된 암구어를 알려주기위해
    if (WrongCodeWord >= CodeWord)
    {
        WrongCodeWord++;
    }

    switch (CodeWord)
    {
        case 0: return TEXT("앞코"); break;
        case 1: return TEXT("뒷굽"); break;
        case 2: return TEXT("부츠끈"); break;
        case 3: return TEXT("혀"); break;
        case 4: return TEXT("목통"); break;
        case 5: return TEXT("밑창"); break;
        case 6: return TEXT("덧창"); break;
        case 7: return TEXT("깔창"); break;
        case 8: return TEXT("안창"); break;
        case 9: return TEXT("장식못"); break;
        default: return ""; break;
    }
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
            TSharedPtr<FEventInfo> temp = MakeShared<FEventInfo>();
            temp->EventGuest = NewGuest;
            arrWaitingEventList.Add(temp);
            arrEventGuestController.Add(GetWorld()->SpawnActor<AAI_Hotel_Guest_Default>());
        }
    }
}
void UHotel_Manager::SettingEvent()
{
    int nRandRange = arrEventFuntionList.Num() - 1;
    int nEventGuestNum;
    nEventGuestNum = 16;//테스트용 nEventGuestNum = EnviromentLevel * 4 + 4;

   // UE_LOG(LogTemp, Warning, TEXT("세팅 이벤트 넘 : %d"), EnviromentLevel);
    int nAssignmentEventNum = 0;
    for (const auto& TempEvent : arrWaitingEventList)
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
            //UE_LOG(LogTemp, Warning, TEXT("세팅 이벤트 노말 게스트"));
            TempEvent->isNormalGuestEvent = true; //일반 손님
        }
    }

    for (int i = 0; i < arrWaitingEventList.Num(); ++i)
    {
        CheckExecuteFunctionTiming(FET_Assignment, arrWaitingEventList[i]->EventGuest);
    }

    SettingReservationGuest(); //예약자 이름까지 설정후
    Algo::RandomShuffle(arrWaitingEventList); // 랜덤으로 섞기
    Hotel_Clock->StartGameClock();

    Hotel_CCTV->arrCameras[0]->SetCameraState(true);
}

void UHotel_Manager::TryCalling(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum)
{
    //CalledPhone 전화를 건 전화기 , NowCallingPhoneNum 전화기가 현재 걸고있는 전화번호
    FString tempTriggerStr = "WalkerTryCall_" + CalledPhone->RegistPhoneNumber.ToString() + "_To_" + NowCallingPhoneNum.ToString();
    OnEventTriggerAction(FName(tempTriggerStr));

    if (NowCallingPhoneNum == "205" || NowCallingPhoneNum == "305" || CalledPhone->RegistPhoneNumber == "205" || CalledPhone->RegistPhoneNumber == "305") //205/305호의 전화기로 전화를 걸거나 205/305호의 번호를 거는건 실패
    {
        if(NowCallingPhoneNum == "205" && CalledPhone->RegistPhoneNumber == "205") return //205호 이벤트를 위해 벽열기 조건 완료시 신호를 연결중 신호로 변경
        CalledPhone->ConnectFail();
        return;
    }
    else if (NowCallingPhoneNum == "911") //보안팀한테 전화걸기
    {
        Department_Operator->AddDialogueDataState("경고", EDialogueState::DS_Security);
        Department_Operator->SetGuestDialogueDataLast(true);
    }

    if (mapRegistedPhone.Contains(NowCallingPhoneNum))
    {
        AHotel_Phone* CallingPhone = *mapRegistedPhone.Find(NowCallingPhoneNum);
        if (IsValid(CallingPhone) && !IsValid(CallingPhone->aConnectedPhone) && CalledPhone != CallingPhone)
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

void UHotel_Manager::MinusHRScore(int MinusScore, FString Reason)
{ 
    nHumanResourcesScore -= MinusScore;
    FHRRecord tempRecord;
    tempRecord.Minus = true;
    tempRecord.Reason = Reason;
    tempRecord.Score = MinusScore;
    arrHRRocord.Add(tempRecord);
    if (nHumanResourcesScore < 0)
    {
        nHumanResourcesScore = 0; //점수가 마이너스가 되진 않도록
        return;
    }


    UE_LOG(LogTemp, Warning, TEXT("%s"), *Reason);
    if (IsGameEndPhase)
    {
        return;
    }

    if (WalkerNowLocation == "Counter")
    {
        CheckManagerCallingForHR();
    }
}
void UHotel_Manager::CheckManagerCallingForHR()
{
    if (!IsManagerFireWalker && nHumanResourcesScore <= 0)
    {
        Department_Operator->AddDialogueDataState("해고", EDialogueState::DS_Manager_Fire);
        Department_Operator->SetGuestDialogueDataLast(true);
        Department_Operator->EraseDialgueDataState(EDialogueState::DS_Manager_Warning);
        Department_Operator->aPhone->TryCalling("0");
        IsManagerFireWalker = true;
        IsManagerWarningWalker = true; //해고전화는 안함
    }
    else if (!IsManagerWarningWalker && nHumanResourcesScore <= 50)
    {
        Department_Operator->AddDialogueDataState("경고", EDialogueState::DS_Manager_Warning);
        Department_Operator->SetGuestDialogueDataLast(true);
        Department_Operator->aPhone->TryCalling("0");
        IsManagerWarningWalker = true;
    }
}

void UHotel_Manager::PlusHRScore(int plusScore, FString Reason)
{
    nHumanResourcesScore += plusScore;
    FHRRecord tempRecord;
    tempRecord.Minus = false;
    tempRecord.Reason = Reason;
    tempRecord.Score = plusScore;
    arrHRRocord.Add(tempRecord);
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

void UHotel_Manager::AddRegistedRoom(FName Name, AHotel_Guest_Room* Room)
{
    if (Name.IsNone() || !IsValid(Room)) return;
    TUniquePtr<FRoomInfo> TempInfo = MakeUnique<FRoomInfo>();
    TempInfo->Room = Room;
    mapRegistedRoom.Add(Name, MoveTemp(TempInfo));
}

void UHotel_Manager::AddHotelMesh(FName Name, UHotel_StaticMesh* Mesh)
{
    if (Name.IsNone() || !IsValid(Mesh)) return;
    mapHotelMesh.Add(Name, Mesh);
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
        if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find(WalkerNowLocation)) //워커 위치가 바뀌었을때 이벤트 실행
        {
            if (Tempmap->IsValid()) //플레이어가 방에 들어왔을때
            {
                if (IsValid((*Tempmap)->RoomGuest)) //해당방에 게스트가 있을경우
                {
                    (*Tempmap)->RoomGuest->SetLookingPlayer(true); //게스트는 들어온 플레이어를 쳐다 본다
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
            if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find(RoomNum))
            {
                if (Tempmap->IsValid() && !IsValid((*Tempmap)->RoomGuest))  //방이 비었다면
                {
                    (*Tempmap)->RoomGuest = Guest;
                    TempAI->AssigningGuestRoom((*Tempmap)->Room);
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
    if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap->IsValid())
        {
            if (!IsValid((*Tempmap)->RoomGuest)) 
            {
                return true;//체크인 가능 상태에서만 true
            }
        }
    }
    return false; //나머지 모든 경우에서는 false
}

void UHotel_Manager::CheckOutGuestRoom(FName RoomNum)
{
    if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap->IsValid())
        {
            CheckExecuteFunctionTiming(FET_CheckOut, (*Tempmap)->RoomGuest);
            KeyTray->ReturnRoomKey(RoomNum);
            (*Tempmap)->RoomGuest = NULL;
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
    if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap->IsValid())
        {
            if (IsValid((*Tempmap)->RoomGuest))
            {
                (*Tempmap)->RoomGuest->HearingKnockAction();
            }
        }
    }
}
bool UHotel_Manager::CheckCorrectGuest(FName RoomNum, AHotel_Guest* Guest)
{
    if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find(RoomNum))
    {
        if (Tempmap->IsValid())
        {
            if (IsValid((*Tempmap)->RoomGuest) && Guest == (*Tempmap)->RoomGuest)
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
    FEventInfo* temp = arrWaitingEventList[nNowExcutingEvent].Get();

    if (temp)
    {
        if (!temp->isNormalGuestEvent)
        {
            if (!temp->FunctionInfo.isExcutingOnlyEvent)
            {
                arrExecutingEventList.Add(temp);
            }

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
}

void UHotel_Manager::CheckExecuteFunctionTiming(EFunctionExcuteTiming nowTiming, AHotel_Guest* TargetGuest)
{
    for (auto& temp : arrExecutingEventList)
    {
        if (temp->isAreadyExcute) continue; //이미 실행된 이벤트면 넘기고
        if (temp->EventGuest != TargetGuest) continue; //대상 게스트가 이벤트의 게스트가 아니면 넘기고
        if (temp->FunctionInfo.ExecuteTiming == nowTiming)
        {
            temp->FunctionInfo.ExecuteFunction(temp);
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
            if (!IsValid(temp.Value->RoomGuest))
            {
                int rand = FMath::RandRange(1, 100);
                if (rand < 10)
                {
                    AddOutbreakEventList(guest, FName("Allocate_RoomGuest"));
                }
                else if (rand < 20)
                {
                    if (!arrReservationGuest.Find(guest->GuestName))
                    {
                        AddOutbreakEventList(guest, FName("RequestRejectCheckIn"));
                    }
                }
                    break;
            }
        }
    }
}

bool UHotel_Manager::OnEventTriggerAction(FName triggerName)
{
    bool isTriggeredThisAction = false;
    if (arrExecutingEventList.Num() > 0)
    {
        for (int i = arrExecutingEventList.Num() - 1; i >= 0; --i)//반복문 진행중 클리어되서 삭제되는 이벤트가 있으므로 역순으로 돌아 인덱스 문제 없애기
        {
            if (arrExecutingEventList[i])
            {
                bool CheckClear = arrExecutingEventList[i]->FunctionInfo.CheckClearFunction(arrExecutingEventList[i], triggerName);
                if (!arrExecutingEventList[i]->CollectedTriggers.IsEmpty())
                {
                    if (arrExecutingEventList[i]->CollectedTriggers.Last() == triggerName)
                    {
                        isTriggeredThisAction = true; //트리거 콜렉션 체크해서 마지막이 현재 트리거이름이라면 방금 행동으로 트리거 추가된것.
                    }
                }
                if (CheckClear)
                {
                    RemoveExecutingEvent(arrExecutingEventList[i]);
                }
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
        if (IsValid(temp.Value->RoomGuest)) //이미 체크인된 방
        {
            if (temp.Value->RoomGuest->IsHanging) return;//목이 매달린상태면 리턴
            FString tempRoomTrigger = temp.Value->Room->RoomNumber.ToString() +"Door_Open";
            if (FName(tempRoomTrigger) == TriggerName)
            {
                temp.Value->RoomGuest->GetAIController()->CloseRoomDoor(10.0f); // 문만 열려있음 10초뒤에 닫으러 간다
                temp.Value->RoomGuest->AddDialogueDataState(TEXT("무단 침입에 관하여"), EDialogueState::DS_Guest_TresPass, 0);
                temp.Value->RoomGuest->SetGuestDialogueDataLast(false);
                temp.Value->RoomGuest->OpenConversationUI();
                temp.Value->RoomGuest->SetLookingPlayer(true);
                MinusHRScore(10, TEXT("객실 무단 침입 시도"));
            }

            tempRoomTrigger = "Walker_Enter_" + temp.Value->Room->RoomNumber.ToString();
            if (FName(tempRoomTrigger) == TriggerName) //플레이어가 방에 들어 갔다면 AI타이머 멈춘다.
            {
                temp.Value->RoomGuest->GetAIController()->StopAITimer();
            }

            tempRoomTrigger = "Walker_Out_" + temp.Value->Room->RoomNumber.ToString();
            if (FName(tempRoomTrigger) == TriggerName) //플레이어가 방나간뒤 문닫기 시작
            {
                temp.Value->RoomGuest->GetAIController()->CloseRoomDoor(5.0f);
            }
        }
    }
}
void UHotel_Manager::AddOutbreakEventList(AHotel_Guest * guest , FName eventName)
{
    TSharedPtr<FEventInfo> tempSharedPtr = MakeShared<FEventInfo>();
    arrOutbreakEventList.Add(tempSharedPtr);

    FEventInfo* temp = tempSharedPtr.Get();
    temp->EventGuest = guest;
    arrWaitingEventList.Add(tempSharedPtr);
    temp->FunctionInfo = *mapOutbreakEventFuntion.Find(eventName);
    temp->isNormalGuestEvent = false;

    if (!temp->FunctionInfo.isExcutingOnlyEvent) arrExecutingEventList.Add(temp);
    if (!arrExecutingEventList.IsEmpty())
    {
        CheckExecuteFunctionTiming(FET_Init, arrExecutingEventList.Last()->EventGuest);
        if (WalkerNowLocation == "Counter")
        {
            CheckExecuteFunctionTiming(FET_WalkerEnterCounter, arrExecutingEventList.Last()->EventGuest);
        }
    }
}

void UHotel_Manager::RemoveExecutingEvent(FEventInfo* TartgetEvent)
{
    GetWorld()->GetTimerManager().ClearTimer(TartgetEvent->EventTimer); //타이머 정지
    if (arrExecutingEventList.Contains(TartgetEvent))
    {
        int tempIndex = arrExecutingEventList.IndexOfByKey(TartgetEvent);
        arrExecutingEventList.RemoveAt(tempIndex); //진행중 이벤트 목록에서 제거
    }
}

void UHotel_Manager::Execute_Event_Open205(FEventInfo* eventInfo)
{
    eventInfo->isAreadyExcute = true;
    UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("205Wall");

    if (IsValid(tempMesh))
    {
        tempMesh->SetMeshState(false);
    }

    if (mapRegistedRoom.Contains("205"))
    {
        if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find("205"))
        {
            ActivateGuest(eventInfo->EventGuest, false);
            GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, this, &UHotel_Manager::Fail_Event_Open205, 300);
            (*Tempmap)->Room->HangingNeck(eventInfo->EventGuest);
            (*Tempmap)->Room->SetRoomFlickingLight(2);
            (*Tempmap)->Room->SetActorHiddenInGame(false);
        }
    }
}

bool UHotel_Manager::CheckClear_Event_Open205(FEventInfo* eventInfo, FName TriggerName)
{
    if (TriggerName == "Walker_Enter_205")
    {
        UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("205Wall");
        if (IsValid(tempMesh))
        {
            tempMesh->SetMeshState(true);
        }

        if (mapRegistedRoom.Contains("205"))
        {
            if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find("205"))
            {
                (*Tempmap)->Room->aDoor->SetOpenDoor(false);
                (*Tempmap)->Room->aDoor->SetUncontrolableLock(true);
            }
        }
        UpdateDefualtLevelMenual(eventInfo->FunctionInfo.EventID);
        eventInfo->CollectedTriggers.Add(TriggerName);
        GetWorld()->GetTimerManager().ClearTimer(eventInfo->EventTimer); // 방에 들어가면 실패 타이머 멈추기
    }
    else if (TriggerName == "WalkerTryCall_205_To_205" && !eventInfo->CollectedTriggers.IsEmpty() && eventInfo->CollectedTriggers.Last() == "Walker_Enter_205")
    {
        UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("205Wall");
        if (IsValid(tempMesh))
        {
            tempMesh->SetMeshState(false);
        }
        if (mapRegistedRoom.Contains("205"))
        {
            if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find("205"))
            {
                (*Tempmap)->Room->aDoor->SetUncontrolableLock(false);
                (*Tempmap)->Room->aDoor->SetOpenDoor(true);
            }
        }
        eventInfo->CollectedTriggers.Add(TriggerName);
    }
    else if (TriggerName == "Walker_Enter_2F" && !eventInfo->CollectedTriggers.IsEmpty() && eventInfo->CollectedTriggers.Last() == "WalkerTryCall_205_To_205" )
    {
        UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("205Wall");
        if (IsValid(tempMesh))
        {
            tempMesh->SetMeshState(true);
        }
        if (mapRegistedRoom.Contains("205"))
        {
            if (TUniquePtr<FRoomInfo>* Tempmap = mapRegistedRoom.Find("205"))
            {
                (*Tempmap)->Room->aDoor->SetOpenDoor(false);
                (*Tempmap)->Room->aDoor->SetUncontrolableLock(true);
            }
        }
        return true;
    }
    else if (TriggerName == "GameEnd")
    {
        Fail_Event_Open205();
    }
    return false;
}
void UHotel_Manager::Fail_Event_Open205()
{
    UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("205Wall");

    if (IsValid(tempMesh))
    {
        tempMesh->SetMeshState(true);
    }
    MinusHRScore(20, TEXT("205호 격리 절차 미이행"));
}


void UHotel_Manager::Execute_Event_StarePeopleUnderLight(FEventInfo* eventInfo)
{
    eventInfo->isAreadyExcute = true;
    ActivateGuest(eventInfo->EventGuest, false);
    eventInfo->EventGuest->IsWierdStareUnderLight = true; 
    eventInfo->EventGuest->TeleportTo(FVector(-373, -100, 17), StreetLight->GetActorRotation());
    eventInfo->EventGuest->SetAutoDoor(AutoDoor);
    eventInfo->EventGuest->SetLookingPlayer(true);
}

void UHotel_Manager::LookingTiemOver_StarePeopleUnderLight()
{
    if (mapRegistedPlace.Contains("Lobby"))
    {
        AHotel_Place* TempPlace = *mapRegistedPlace.Find("Lobby");
        if (IsValid(TempPlace) && IsValid(TempPlace->PlaceSwtich))
        {
            TempPlace->PlaceSwtich->EventLightAction("FlickingOnce");
        }
        StreetLight->FlickeringOnce();
        OnEventTriggerAction("NoLookAtGuest");
    }
}
bool UHotel_Manager::CheckClear_Event_StarePeopleUnderLight(FEventInfo* eventInfo, FName TriggerName)
{
    if (TriggerName == "SecurityReport_0" && eventInfo->CollectedTriggers.Num() > 5) //호텔에 들어왔을때 보안팀 호출시
    {
        eventInfo->CollectedTriggers.Add(TriggerName);
    }
    else if (TriggerName == "LookAtGuest")
    {
        if (!eventInfo->CollectedTriggers.IsEmpty() && eventInfo->CollectedTriggers.Last() == "SecurityReport_0") return false; //이미 클리어 조건 갖춰졌을땐 아무것도 안한다
        if (eventInfo->CollectedTriggers.IsEmpty() || eventInfo->CollectedTriggers.Last() != TriggerName)
        {
            eventInfo->CollectedTriggers.Add(TriggerName);
        }

        GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, this, &UHotel_Manager::LookingTiemOver_StarePeopleUnderLight, FMath::RandRange(10, 30));
    }
    else if (TriggerName == "NoLookAtGuest")
    {
        if (eventInfo->CollectedTriggers.Last() == "SecurityReport_0")
        {
            eventInfo->EventGuest->TeleportTo(FVector(-327, 839, 94), FRotator(0, 0, 0),false,false);//호텔 밖
            return true;
        }

        GetWorld()->GetTimerManager().ClearTimer(eventInfo->EventTimer);
        if (eventInfo->EventGuest->bWasLookingAtGuest) eventInfo->EventGuest->bWasLookingAtGuest = false;       //타이머로 트리거 강제 발동시 시야 판정을 다시 하기위해 false로
        if ((!eventInfo->CollectedTriggers.IsEmpty() && eventInfo->CollectedTriggers.Last() != TriggerName))
        {
            eventInfo->CollectedTriggers.Add(TriggerName);
        }
        switch (eventInfo->CollectedTriggers.Num())
        {
            case 2:
            {
                StreetLight->SetSoftFlickering();
            }
            break;

            case 4:
            {
                eventInfo->EventGuest->TeleportTo(FVector(-770, 1111, 96), FRotator(0.0f, 0.0f, 0.0f)); // 문앞
                StreetLight->SetFlickeringOnce();
            }
            break;
        
            case 6:
            {
                if (WalkerNowLocation == "Counter")
                {
                    eventInfo->EventGuest->TeleportTo(FVector(-1139, 2063, 96), FRotator(0.0f, 0.0f, 0.0f)); // 카운터 앞
                }
                else
                {
                    eventInfo->EventGuest->TeleportTo((Hotel_Walker->FollowCamera->GetComponentLocation() + (Hotel_Walker->FollowCamera->GetForwardVector() * 50)), FRotator(0.0f, 0.0f, 0.0f), false, false); // 캐릭터 정면
                }
            }
            break;
            case 8:
            {
                UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
                if (NavSys && eventInfo->EventGuest->Controller && eventInfo->EventGuest->Controller->GetPawn())
                {
                    FVector StartLocation = eventInfo->EventGuest->Controller->GetPawn()->GetActorLocation();
                    FVector EndLocation = Hotel_Walker->GetActorLocation(); // 이동 목표 지점
                    UNavigationPath* NavPath = NavSys->FindPathToLocationSynchronously(GetWorld(), StartLocation, EndLocation);

                    for (int32 i = 0; i < NavPath->PathPoints.Num(); ++i)
                    {
                        DrawDebugSphere(GetWorld(), NavPath->PathPoints[i], 20.0f, 8, FColor::Green, false, 5.0f);

                        if (i > 0)
                        {
                            DrawDebugLine(GetWorld(), NavPath->PathPoints[i - 1], NavPath->PathPoints[i], FColor::Blue, false, 5.0f);
                        }
                    }
                    if (NavPath && NavPath->IsValid() && NavPath->PathPoints.Num() >= 3)
                    {
                        FVector PointBeforeGoal = NavPath->PathPoints[NavPath->PathPoints.Num() - 2];
                        PointBeforeGoal.Z += 90.0f;
                        bool bCanTeleport = eventInfo->EventGuest->TeleportTo(PointBeforeGoal, FRotator(0.0f, 0.0f, 0.0f), false, true); // 캐릭터 앞
                    }
                    else
                    {
                        eventInfo->EventGuest->TeleportTo((Hotel_Walker->FollowCamera->GetComponentLocation() + (Hotel_Walker->FollowCamera->GetForwardVector() * -50)), FRotator(0.0f, 0.0f, 0.0f), false, true); // 캐릭터 카메라 뒤로 50 거리에
                    }
                }
            }
            break;
            case 10:
            {
                eventInfo->EventGuest->CatchingPlayer();
                UpdateDefualtLevelMenual(eventInfo->FunctionInfo.EventID);
                if (mapRegistedPlace.Contains("Lobby"))
                {
                    AHotel_Place* TempPlace = *mapRegistedPlace.Find("Lobby");
                    if (IsValid(TempPlace) && IsValid(TempPlace->PlaceSwtich))
                    {
                        TempPlace->PlaceSwtich->EventLightAction("FlickingHard");
                    }
                }
            }
            break;
        }
    }
    return false;
}

void UHotel_Manager::Execute_Event_Invader(FEventInfo* eventInfo)
{
    eventInfo->isAreadyExcute = true;
    ActivateGuest(eventInfo->EventGuest, false);
    eventInfo->EventGuest->IsReadyToNeckShaking = true;
    //이벤트 시작시 Staff 도어의 현재 상태에 따라 트리거 실행
    StaffDoor->GetLockState() ? OnEventTriggerAction("StaffDoor_Lock") : OnEventTriggerAction("StaffDoor_Unlock");

}

void UHotel_Manager::UnlockTiemOver_Invader()
{
    OnEventTriggerAction("InvadeInvader");
}

bool UHotel_Manager::CheckClear_Event_Invader(FEventInfo* eventInfo, FName TriggerName)
{
    if (!eventInfo->CollectedTriggers.IsEmpty() && eventInfo->CollectedTriggers.Last() == "InvadeInvader" )  //이미 침입자가 활성화 되었을때
    {
        if (TriggerName == "SecurityReport_0")//침입자 침입후 보안팀 카운터 호출시
        {
            eventInfo->EventGuest->TeleportTo(FVector(0, 1343, 94), FRotator(0, 0, 0), false, false);//호텔 밖
            eventInfo->CollectedTriggers.Add(TriggerName);
            UpdateDefualtLevelMenual(eventInfo->FunctionInfo.EventID);
            return true;
        }
        else if (WalkerNowLocation == "StaffRoom" || TriggerName == "StaffDoor_Open") //스태프룸에 들어가있다면
        {
            eventInfo->EventGuest->CatchingPlayer();
            UpdateDefualtLevelMenual(eventInfo->FunctionInfo.EventID);
        }
    }
    else if(TriggerName == "StaffDoor_Lock")
    {
        if (!eventInfo->CollectedTriggers.IsEmpty() && eventInfo->CollectedTriggers.Last() == "InvadeInvader") return false; //이미 침입자가 활성화 되었다면 아무것도 하지않는다
        UE_LOG(LogTemp, Warning, TEXT("침입 타이머 OFF"));
        GetWorld()->GetTimerManager().ClearTimer(eventInfo->EventTimer);
        eventInfo->CollectedTriggers.Add(TriggerName);
    }
    else if (TriggerName == "StaffDoor_Unlock" )
    {
        if (!eventInfo->CollectedTriggers.IsEmpty() && (eventInfo->CollectedTriggers.Last() == "InvadeInvader" || eventInfo->CollectedTriggers.Last() == "StaffDoor_Unlock")) return false; //이미 침입자가 활성화 되었다면 아무것도 하지않는다
        UE_LOG(LogTemp, Warning, TEXT("침입 타이머 ON"));
        GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, this, &UHotel_Manager::UnlockTiemOver_Invader, FMath::RandRange(10, 15));
        eventInfo->CollectedTriggers.Add(TriggerName);
    }
    else if (TriggerName == "InvadeInvader")
    {
        eventInfo->CollectedTriggers.Add(TriggerName);
        UE_LOG(LogTemp, Warning, TEXT("침입자 등장"));

        if (WalkerNowLocation == "Counter")
        {
            StaffDoor->SetOpenDoor(false);
            eventInfo->EventGuest->ReadyToRotate(); // 컨트롤러의 회전을 따름
            bool bCanTeleport = eventInfo->EventGuest->TeleportTo(FVector(-1222, 2549, 96), FRotator(0.0f, 180.0f, 0.0f), false, true);
        }
        else
        {
            StaffDoor->SetOpenDoor(false);
            bool bCanTeleport = eventInfo->EventGuest->TeleportTo(FVector(-744, 2594.0, 96), FRotator(0.0f, 0.0f, 0.0f), false, true);
        }
    }
    return false;
}

void UHotel_Manager::Execute_Only_MakeDirtyRoom(FEventInfo* eventInfo)
{
    for (auto& temp : mapRegistedRoom)
    {
        if (!IsValid(temp.Value->RoomGuest))
        {
            temp.Value->Room->MakeRoomDirty(true);
            temp.Value->Room->aDoor->SetLockDoor(false);
            temp.Value->Room->aDoor->SetOpenDoor(true);
            return;
        }
    }
}

void UHotel_Manager::ExecuteEvent_Guest_LostSignalCCTV(FEventInfo* eventInfo)
{
    UE_LOG(LogTemp, Warning, TEXT("로스트 시그널 게스트"));
    eventInfo->isAreadyExcute = true;
    ActivateGuest(eventInfo->EventGuest, true);
    eventInfo->EventGuest->IsWierdFaceLook = true;
    eventInfo->EventGuest->IsCheckTrigger = true;
}

bool UHotel_Manager::CheckClear_Guest_LostSignalCCTV(FEventInfo* eventInfo, FName triggerName)
{
    FString TriggerString = triggerName.ToString();
    TArray<FString> Result;
    TriggerString.ParseIntoArray(Result, TEXT("_"),true);

    if (eventInfo->EventGuest->GuestName != Result[0]) return false; //다른 손님의 트리거면 리턴
    if (Result[1] == "OutHotel") //호텔 퇴실시 로비카메라 끄기
    {
        Hotel_CCTV->SetPostProcessCamera(0, 3, true);

    }
    if (Result[1] == "In")
    {
        UE_LOG(LogTemp, Warning, TEXT("로스트 시그널 게스트 %s"), *triggerName.ToString());
        int CameraNum = Hotel_CCTV->FindCameraNumByName(FName(Result[2]));
        if (CameraNum == -1) return false;    //장소가 cctv에 없는 장소면
        Hotel_CCTV->SetPostProcessCamera(CameraNum, 3, true);
    }
    else if (Result[1] == "Out")
    {
        UE_LOG(LogTemp, Warning, TEXT("로스트 시그널 게스트 %s"), *triggerName.ToString());
        int CameraNum = Hotel_CCTV->FindCameraNumByName(FName(Result[2]));
        if (CameraNum == -1) return false;    //장소가 cctv에 없는 장소면
        Hotel_CCTV->SetPostProcessCamera(CameraNum, 3, false);
    }
    return false;
}

void UHotel_Manager::ExecuteEvent_Guest_RoomCCTV(FEventInfo* eventInfo)
{
    eventInfo->isAreadyExcute = true;
    ActivateGuest(eventInfo->EventGuest, true);
    eventInfo->EventGuest->IsCheckTrigger = true;
}

bool UHotel_Manager::CheckClear_Guest_RoomCCTV(FEventInfo* eventInfo, FName triggerName)
{
    if (!eventInfo->EventGuest->GetAIController()->AssignedGuestRoom) return false; //입실이 안됐다면
    AHotel_Guest_Room* GuestRoom = eventInfo->EventGuest->GetAIController()->AssignedGuestRoom;
    FString RoomNumber = GuestRoom->RoomNumber.ToString();
    UE_LOG(LogTemp, Warning, TEXT("트리거 %s"), *triggerName.ToString());
    if (FName(eventInfo->EventGuest->GuestName + "_In_" + RoomNumber) == triggerName)
    {
        GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, [this, eventInfo]() {
            Hotel_CCTV->ActiveRoomCameraToCCTV(eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->aCCTV_Camera); 
            }, 1, false);

        GuestRoom->SetIsolationState(true);
    }
    FString TriggerString = triggerName.ToString();
    TArray<FString> Result;
    TriggerString.ParseIntoArray(Result, TEXT("_"), true);

    if (eventInfo->CollectedTriggers.Num() < 7)
    {
        if (triggerName == "Walker_Out_Counter") // 방 cctv를 틀어놓고 카운터 나갈시
        {

            if (Hotel_CCTV->GetRoomCameraIndex(GuestRoom->aCCTV_Camera->CameraName) == Hotel_CCTV->GetNowCameraNum())
            {
                GetWorld()->GetTimerManager().ClearTimer(eventInfo->EventTimer);
                for (int i = eventInfo->CollectedTriggers.Num() ; i < 5 ; i++)
                {
                    eventInfo->CollectedTriggers.Add(FName("ViewCCTV_" + RoomNumber)); 
                    eventInfo->EventGuest->SetLookingCameraStatue(eventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
                }
            }
        }
        else if (triggerName == "Walker_Enter_Counter") // 방 cctv를 틀어놓고 다시 카운터 재입장시 
        {
            if (Hotel_CCTV->GetRoomCameraIndex(GuestRoom->aCCTV_Camera->CameraName) == Hotel_CCTV->GetNowCameraNum())
            {
                GetWorld()->GetTimerManager().ClearTimer(eventInfo->EventTimer);
                eventInfo->CollectedTriggers.Add(triggerName);
                eventInfo->EventGuest->SetLookingCameraStatue(eventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
                UpdateDefualtLevelMenual(eventInfo->FunctionInfo.EventID);
            }
            
        }
        else if (Result[0] == "ViewCCTV")
        {
            if (Result[1] == RoomNumber)
            {
                eventInfo->CollectedTriggers.Add(triggerName);
                eventInfo->EventGuest->SetLookingCameraStatue(eventInfo->CollectedTriggers.Num(), GuestRoom->aCCTV_Camera);
                GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, [this, triggerName]() {OnEventTriggerAction(triggerName); }, 2, false);
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("타이먹 죽여"));
                GetWorld()->GetTimerManager().ClearTimer(eventInfo->EventTimer);
            }
        }
    }
   
    return false;
}

void UHotel_Manager::SettingReservationGuest()
{
    int MaxReservationNum = FMath::RandRange(0, 7); //예약손님은 최대 4명까지 랜덤으로 설정 총 이벤트 갯수는 6개 보단 무조건 많을 예정
    
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

void UHotel_Manager::ExecuteEvent_PeepingPlayer()
{

}

void UHotel_Manager::Execute_Event_Complain_RoomDirty(FEventInfo* eventInfo)
{
    eventInfo->isAreadyExcute = true;
    eventInfo->EventGuest->AddDialogueDataState(TEXT("방 청소 상태에 관하여"), EDialogueState::DS_Guest_ComplainRoomCondition, 0); //대화 상태값 설정
    eventInfo->EventGuest->SetGuestDialogueDataLast(false);
    eventInfo->EventGuest->IsCalledWalker = true; // 이미 전화로 전달 된거 확인
    eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->aPhone->TryCalling("0"); //전화 걸기

}

bool UHotel_Manager::CheckClear_Complain_RoomDirty(FEventInfo* eventInfo, FName triggerName)
{
    FString RoomNumber = eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->RoomNumber.ToString();
   
    if (triggerName == FName(RoomNumber + "_Clean"))
    {
        if (!eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->CheckRoomDirty())
        {
            return true;
        }
    }
    return false;
}


void UHotel_Manager::Execute_Event_Request_Reject_CheckIn(FEventInfo* eventInfo)
{
    eventInfo->isAreadyExcute = true;
    GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, [this , eventInfo]()
        {
            Department_Operator->AddDialogueDataState("요청", EDialogueState::DS_Manager_Request_Reject_CheckIn);
            Department_Operator->SetGuestDialogueDataLast(true);
            Department_Operator->aPhone->TryCalling("0");
        }, FMath::RandRange(2,8), false);
}

bool UHotel_Manager::CheckClear_Request_Reject_CheckIn(FEventInfo* eventInfo, FName triggerName)
{
    FString TriggerString = triggerName.ToString();
    TArray<FString> Result;
    TriggerString.ParseIntoArray(Result, TEXT("_"), true);

    if (Result[0] != eventInfo->EventGuest->GuestName) return false;
    if (Result[1] == "CheckIn")
    {
        MinusHRScore(20, TEXT("지시 불이행"));

        Department_Operator->AddDialogueDataState("요청", EDialogueState::DS_Manager_Request_Reject_CheckIn);
        Department_Operator->SetGuestDialogueDataLast(true);

        return true;
    }
    else if (Result[1] == "OutHotel")
    {
        return true;
    }

    return false;
}

void UHotel_Manager::Execute_Event_Allocate_Room(FEventInfo* eventInfo)
{
    TArray<FString> arrEmptyRoom;
    for (auto& temp : mapRegistedRoom)
    {
        if (!IsValid(temp.Value->RoomGuest))
        {
            if (temp.Value->Room->RoomNumber != "205" && temp.Value->Room->RoomNumber != "305")
            {
                arrEmptyRoom.Add(temp.Key.ToString());
            }
        }
    }
    eventInfo->CollectedTriggers.Add(FName("AllocateRoomNum_" + arrEmptyRoom[FMath::RandRange(0, arrEmptyRoom.Num() - 1)]));
    GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, [this , eventInfo]()
        {
            Department_Operator->AddDialogueDataState("요청", EDialogueState::DS_Manager_Allocate_Room_Guest);
            Department_Operator->SetGuestDialogueDataLast(true);
            Department_Operator->aPhone->TryCalling("0");
        }, FMath::RandRange(2, 8), false);
}

bool UHotel_Manager::CheckClear_Allocate_Room(FEventInfo* eventInfo, FName triggerName)
{
    FString TriggerString = triggerName.ToString();
    TArray<FString> Result;
    TriggerString.ParseIntoArray(Result, TEXT("_"), true);
    UE_LOG(LogTemp, Warning, TEXT("체크 얼로케이트 룸 %s") , *triggerName.ToString());
    if (Result[0] != eventInfo->EventGuest->GuestName) return false;
    if (Result[1] == "CheckIn")
    {
        if (Result[2] == eventInfo->CollectedTriggers.Last())
        {
            return true;
        }
        else
        {
            MinusHRScore(20, TEXT("지시 불이행"));
            return true;
        }
    }
    else if (Result[1] ==  "OutHotel")
    {
        MinusHRScore(20, TEXT("지시 불이행"));
        return true;
    }

    return false;
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

void UHotel_Manager::ExecuteEvent_Guest_Hanging(FEventInfo* eventInfo)
{
    UE_LOG(LogTemp, Warning, TEXT("행잉 게스트"));
    eventInfo->isAreadyExcute = true;
    ActivateGuest(eventInfo->EventGuest, true);
    eventInfo->EventGuest->IsCheckTrigger = true;
    eventInfo->EventGuest->IsWierdHanging = FMath::RandRange(0, 1) == 0 ? false : true;// 반반 확률로 눈뜨고 죽은 손님
}

bool UHotel_Manager::CheckClear_Guest_Hanging(FEventInfo* eventInfo, FName triggerName)
{
    if (!eventInfo->EventGuest->GetAIController()->AssignedGuestRoom) return false;
    if (FName(eventInfo->EventGuest->GuestName + "_CheckIn") == triggerName)
    {
        UE_LOG(LogTemp, Warning, TEXT("행잉 게스트 1"));
        eventInfo->CollectedTriggers.Add(triggerName);
    }
    if (FName(eventInfo->EventGuest->GuestName + "_OutHotel") == triggerName)
    {
        return true;// 어떤 이유로든 호텔 퇴실 시 클리어
    }


    if (eventInfo->CollectedTriggers.Num() == 0) return false; //체크인이 선행이 되야 객실 입장 체크
    FString RoomNumber = eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->RoomNumber.ToString();

    if (eventInfo->EventGuest->IsWierdHanging)
    {
        if (FName("SecurityReport_"+ RoomNumber) == triggerName)    //이상 손님일시 신고까지 해야 클리어
        {
            eventInfo->EventGuest->SetActorHiddenInGame(true);
            eventInfo->EventGuest->TeleportTo(FVector(-327, 839, 94), FRotator(0, 0, 0));
            eventInfo->EventGuest->CheckOutGuest();
            UpdateDefualtLevelMenual(eventInfo->FunctionInfo.EventID);
            return true;
        }
    }
    else
    {
        if (FName(eventInfo->EventGuest->GuestName + "_HangingNeck") == triggerName)
        {
            return true;
        }
    }

    FName CheckTriggerName = FName(eventInfo->EventGuest->GuestName + "_In_" + RoomNumber);
    if (CheckTriggerName == triggerName)
    {
        int RandTime = FMath::RandRange(1,10);
        GetWorld()->GetTimerManager().SetTimer(eventInfo->EventTimer, [eventInfo]()
            {
                eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->SetRoomFlickingLight(1);
                eventInfo->EventGuest->GetAIController()->AssignedGuestRoom->HangingNeck(eventInfo->EventGuest);
                eventInfo->EventGuest->GetAIController()->StopAITimer();
                eventInfo->EventGuest->GetAIController()->StopPatientTimer();
            }, 3.0f, false);
    }
    return false;
}
/*

void UHotel_Manager::Execute_Event_OpenBasement(FEventInfo* eventInfo)
{
    UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("BasemantEntrance");
    if (IsValid(tempMesh))
    {
        tempMesh->SetVisibility(false);
    }
}
void UHotel_Manager::Execute_Event_Open305(FEventInfo* eventInfo)
{
    UHotel_StaticMesh* tempMesh = *mapHotelMesh.Find("305Wall");
    if (IsValid(tempMesh))
    {
        tempMesh->SetVisibility(false);
    }
}
*/

void UHotel_Manager::SaveLevelOption(int menual, int Enviroment)
{
    EnviromentLevel = Enviroment;
    MenualLevel = menual;
    SaveGameLevelOption();
}

void UHotel_Manager::SaveGameLevelOption()
{
    if (!HotelSaveGameOption) return;

    // 옵션 저장
    HotelSaveGameOption->MenualLevel = MenualLevel;
    HotelSaveGameOption->EnviromentLevel = EnviromentLevel;
    HotelSaveGameOption->arrExperiencedEventID = arrExperiencedEventID;
    // 실제 디스크 저장                 
    if (HotelSaveGameOption && IsValid(HotelSaveGameOption))
    {
      // UGameplayStatics::SaveGameToSlot(HotelSaveGameOption, TEXT("PlayerSaveSlot"), 0);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("HotelSaveGame is nullptr!"));
    }
}

void UHotel_Manager::LoadGameLevelOption()
{
    if (UGameplayStatics::DoesSaveGameExist(TEXT("PlayerSaveSlot"), 0))
    {
        HotelSaveGameOption = Cast<UHotelSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PlayerSaveSlot"), 0));

        UE_LOG(LogTemp, Warning, TEXT("로드 완료"));
        MenualLevel = HotelSaveGameOption->MenualLevel;
        EnviromentLevel = HotelSaveGameOption->EnviromentLevel;
        arrExperiencedEventID = HotelSaveGameOption->arrExperiencedEventID;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("로드 실패"));
        HotelSaveGameOption = Cast<UHotelSaveGame>(UGameplayStatics::CreateSaveGameObject(UHotelSaveGame::StaticClass()));
        MenualLevel = 1;
        EnviromentLevel = 1;
        arrExperiencedEventID.Empty();
    }
}

bool UHotel_Manager::LoadGameManualExternal()
{
    FString SaveDir = FPaths::ProjectSavedDir();
    FString FullPath = SaveDir / MenualSaveName;

    FString FileData;
    if (FFileHelper::LoadFileToString(FileData, *FullPath))
    {
        UE_LOG(LogTemp, Log, TEXT("불러온 텍스트:\n%s"), *FileData);
        FioneerMenualText = FileData;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("파일을 불러올 수 없음: %s"), *FullPath);
        return false;
    }

    /* //Json 로드 언젠가 쓰일지도
    FString SaveDir = FPaths::ProjectSavedDir();
    FString FullPath = SaveDir / MenualSaveName;

    UE_LOG(LogTemp, Warning, TEXT("경로 : %s"), *FullPath);
    FString FileData;
    if (!FFileHelper::LoadFileToString(FileData, *FullPath))
    {
        UE_LOG(LogTemp, Warning, TEXT("파일을 불러올 수 없음: %s"), *FullPath);

        return false;
    }
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileData);

    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("JSON 파싱 실패"));
        return false;
    }

    MenualText.Empty(); // 기존 메뉴얼 초기화

    const TArray<TSharedPtr<FJsonValue>>* ManualArray;
    if (RootObject->TryGetArrayField(TEXT("Manuals"), ManualArray))
    {
        for (auto& Value : *ManualArray)
        {
            TSharedPtr<FJsonObject> Obj = Value->AsObject();
            if (Obj.IsValid())
            {
                FManualInfo Info;
                Info.Category = Obj->GetIntegerField(TEXT("Category"));
                Info.EventID = Obj->GetIntegerField(TEXT("EventID"));
                Info.MenualText = FText::FromString(Obj->GetStringField(TEXT("Text")));

                MenualText.Add(Info);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("JSON 메뉴얼 불러오기 완료: %s"), *FullPath);*/
    return true;
}

bool UHotel_Manager::SaveGameManualExternal()
{
    FString SaveDir = FPaths::ProjectSavedDir(); // Saved/ 경로
    FString FullPath = SaveDir / MenualSaveName;       // ex) Saved/ManualNote.txt

    UE_LOG(LogTemp, Warning, TEXT("세이브 경로 %s"), *FullPath);

    FString OutputText= FioneerMenualText;
    UE_LOG(LogTemp, Warning, TEXT("세이브 텍스트 %s"), *FioneerMenualText);

    FFileHelper::SaveStringToFile(OutputText, *FullPath);

    /* //Json 세이브 언젠가 쓰일지도
    FString SaveDir = FPaths::ProjectSavedDir();
    FString FullPath = SaveDir / MenualSaveName;

    TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> ManualArray;

    for (const FManualInfo& Info : MenualText)
    {
        TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetNumberField(TEXT("Category"), Info.Category);
        Obj->SetNumberField(TEXT("EventID"), Info.EventID);
        Obj->SetStringField(TEXT("Text"), Info.MenualText.ToString());

        ManualArray.Add(MakeShared<FJsonValueObject>(Obj));
    }

    RootObject->SetArrayField(TEXT("Manuals"), ManualArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject, Writer);

    FFileHelper::SaveStringToFile(OutputString, *FullPath); 
    */
    return true;
}

void UHotel_Manager::InitMenualInfo()
{
    arrExperiencedEventID.Empty();//초기화
    FioneerMenualText = "";
}

void UHotel_Manager::UpdateDefualtLevelMenual(int EventId)
{
    if (MenualLevel != 1) return;
    if(EventId > 0 &&!arrExperiencedEventID.Find(EventId))
    {
        arrExperiencedEventID.Add(EventId);
        arrExperiencedEventID.Sort();
    }
}

void UHotel_Manager::SetGameEnd(EGameEndReason Reason)
{
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
    FTimerHandle EndGameTimer;

    if (Hotel_Walker)
    {
        Hotel_Walker->HideUIName("InteractBox");
        Hotel_Walker->HideUIName("InteractMessage");
        Hotel_Walker->HideUIName("Menual");
        Hotel_Walker->HideUIName("Dialogue");
        Hotel_Walker->HideUIName("Phone");
        Hotel_Walker->SetActorTickEnabled(false);
    }

    if (Level_Manager)
    {
        Level_Manager->LoadEndingLevel();
    }
    for (auto& temp : arrEventGuestController)
    {
        temp->StopAITimer();
        temp->StopPatientTimer();
    }

}

void UHotel_Manager::SetMainMenu()
{
    Level_Manager->LoadMainLevel();
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
    arrHRRocord.Empty();

    for (auto & temp : arrExecutingEventList)
    {
        GetWorld()->GetTimerManager().ClearTimer(temp->EventTimer);
    }
    arrExecutingEventList.Empty();

    IsContinueSetting = isContinue;
    IsGameEndPhase = false;
    EndReason = GER_NotYet;
    nNowExcutingEvent = 0;

    IsManagerWarningWalker = false;
    IsManagerFireWalker = false;

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
        nHumanResourcesScore = 100;
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


void UHotel_Manager::StartWalkerFireProcess()
{
    for (auto& temp : mapHotelSwitch)
    {
        temp.Value->EventLightAction("HardFlicking");
    }
    Department_Operator->TeleportTo((Hotel_Walker->FollowCamera->GetComponentLocation() + (Hotel_Walker->FollowCamera->GetForwardVector() * -50)), FRotator(0.0f, 0.0f, 0.0f), false, true); // 캐릭터 카메라 뒤로 50 거리에
    Department_Operator->CatchingPlayer();
}