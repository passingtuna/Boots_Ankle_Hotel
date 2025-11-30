// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/Character.h"
#include "DialogueDataAsset.h"
#include "GuestDataAsset.h"
#include "Hotel_Operator.h"
#include "Hotel_CCTV.h"
#include "Hotel_Light.h"
#include "Hotel_StaticMesh.h"
#include "Hotel_Manager.generated.h"

class AHotel_Phone;
class AHotel_Walker;
class AHotel_Guest;
class AHotel_Guest_Room;
class AHotel_Place;
class AHotel_Clock;
class AHotel_KeyTray;
class AHotel_Switch;
class UConversationUI;
class UHotelSaveGame;
class ALevel_Manager;

struct FRoomInfo
{
    AHotel_Guest_Room* Room;
    AHotel_Guest * RoomGuest;
};

struct FGuestname
{
    bool isMan;
    bool isAssigned = false;
    FString Name;
    FString SimilarName;
};
enum EFunctionExcuteTiming
{
    FET_Assignment, FET_Init, FET_CheckIn, FET_EnterRoom, FET_CheckOut ,FET_WalkerEnterCounter
};

enum EGameEndReason
{
    GER_NotYet,GER_Dead,GER_Fired,GER_Clear
};

struct FExcuteFunctionInfo
{
    int EventID;
    EFunctionExcuteTiming ExecuteTiming = FET_Init;
    bool isEventOnlyOnce = false;
    bool isExcutingOnlyEvent = false;//실행만하고 따로 트리거 체크를 하지않아 대기 리스트에 넣지 않음
    TFunction<void(struct FEventInfo* eventInfo)> ExecuteFunction;
    TFunction<bool(struct FEventInfo* eventInfo, FName trigger)> CheckClearFunction;
};

struct FEventInfo
{
    AHotel_Guest* EventGuest;
    FTimerHandle EventTimer;
    bool isNormalGuestEvent = true;
    bool isAreadyExcute = false;
    TArray<FName> CollectedTriggers;
    FExcuteFunctionInfo FunctionInfo;
};

struct FHRRecord
{
    FString Reason;
    bool Minus;
    int Score;
}
;

USTRUCT(BlueprintType)
struct FManualInfo : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText MenualText;
};

UCLASS()
class BOOTS_ANKLE_HOTEL_API UHotel_Manager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
    //-------------------초기화---------------------------------------
private:
    AHotel_CCTV* Hotel_CCTV;
    AHotel_Light* StreetLight;
    AHotel_Door* StaffDoor;
    AActor* AutoDoor;
    AHotel_Clock* Hotel_Clock;
    TArray <FGuestname> arrGuestName;
    TArray<AAI_Hotel_Guest_Default*> arrEventGuestController;
    AHotel_KeyTray* KeyTray;
    AHotel_Walker* Hotel_Walker;
    AHotel_Operator* Department_Operator;
    int CodeWord;
    bool IsAlreadyInit;
    ALevel_Manager* Level_Manager;
    bool IsManagerWarningWalker;
    bool IsManagerFireWalker;

    bool IsGameEndPhase;

    TMap<FName, AHotel_Phone*> mapRegistedPhone;
    TMap<FName, AHotel_Place*> mapRegistedPlace;
    TMap<FName, UDialogueDataAsset*> DialogueMap;
    TMap<FName, TUniquePtr<FRoomInfo>> mapRegistedRoom;
    TMap<FName, UHotel_StaticMesh*> mapHotelMesh;
    TMap<FName, AHotel_Switch*> mapHotelSwitch;

    TArray <UGuestDataAsset*> arrBPGuestDataAsset;

    bool IsContinueSetting;

public:
    void LoadGuestName();
    void SetHotelWalker(AHotel_Walker* Walker) { Hotel_Walker = Walker; };
    void SetKeyTray(AHotel_KeyTray* KeyTrayObj) { KeyTray = KeyTrayObj; };
    void SetHotel_CCTV(AHotel_CCTV* cctv) { Hotel_CCTV = cctv; };
    void SetStreetLight(AHotel_Light* light) { StreetLight = light; };
    void SetStaffDoor(AHotel_Door* door) { StaffDoor = door; };
    void SetHotelClock(AHotel_Clock * clock) { Hotel_Clock = clock; };
    void SetDepartmentOperator(AHotel_Operator* Departmentoperator) { Department_Operator = Departmentoperator; };
    void SetLevelManager(ALevel_Manager* levelManager) { Level_Manager = levelManager; };
    UFUNCTION(BlueprintCallable)
    void SetAutoDoor(AActor* Door) { AutoDoor = Door; };
    void SetOutDoorStreetLight(AHotel_CCTV* cctv) { Hotel_CCTV = cctv; };
    void SpawnGuest();
    void AddRegistPhone(FName Number, AHotel_Phone* Phone);
    void AddRegistedPlace(FName Name, AHotel_Place* Place);
    void AddRegistedRoom(FName Name, AHotel_Guest_Room* Room);
    void AddHotelMesh(FName Name, UHotel_StaticMesh* Mesh);
    void AddHotelSwitch(FName Name, AHotel_Switch* hotelSwitch);

    AHotel_Walker* GetHotelWalker() { return Hotel_Walker; };

    ALevel_Manager* GetLevel_Manager() { return Level_Manager; };
    
    FString GetCodeWord();
    FString GetWrongCodeWord();

    void SettingInitGame(bool isResume);
//------------------게임 진행-----------------------------------
private:
    TArray <FString> arrReservationGuest;
    FTimerHandle CallingTimer;
    FTimerHandle EventTermTimer;
    FName WalkerNowLocation;

    int nHumanResourcesScore;
    TArray <FHRRecord> arrHRRocord;
    EGameEndReason EndReason;

    int nWalkingDay;
public:
    void CheckManagerCallingForHR();
    void CheckInGuestRoom(AHotel_Guest * guest, FName roomNum);
    bool CheckGuestRoom(FName roomNum);
    void CheckOutGuestRoom(FName roomNum);
    void GuestOutHotel(AHotel_Guest * guest);
    void KnockingRoomDoor(FName roomNum);
    void Initialize(FSubsystemCollectionBase& collection)override;
    void TryCalling(AHotel_Phone* calledPhone, FName nowCallingPhoneNum);
    void UpdateWalkerLocation(FName locationName);
    FName GetWalkerLocation() { return WalkerNowLocation; };
    UDialogueDataAsset* GetDialogueData(FName dialogueData);
    void SettingReservationGuest();
    void MinusHRScore(int minusScore, FString reason);
    void PlusHRScore(int plusScore ,FString reason);
    bool CheckCorrectGuest(FName roomNum, AHotel_Guest* guest);
    void WalkerCorpseRetrieval();
    FGuestname GetGenderName(bool gender);
    void ActivateGuest(AHotel_Guest* guest, bool goToCounter);
    void RingingBell();
    void SetGameEnd(EGameEndReason reason);
    EGameEndReason GetGameEnd() { return EndReason; };
    void SetMainMenu();
    int  GetWalkingDay() { return nWalkingDay; };
    void CompleteMainLevelLoad();

    TArray <FHRRecord> GetHRRecord() { return arrHRRocord; };
    int GetHRScore() { return nHumanResourcesScore; };

    bool GetWalkerFired() { return IsManagerFireWalker; };
    FString GetReservationGuestName();
 //-----------------------이벤트----------------------------------
 private:
     int nNowExcutingEvent;
     TArray <FExcuteFunctionInfo> arrEventFuntionList;
     TMap <FName, FExcuteFunctionInfo> mapOutbreakEventFuntion;
     TArray <TSharedPtr<FEventInfo>> arrOutbreakEventList;

     TArray <TSharedPtr<FEventInfo>> arrWaitingEventList;

     TArray <FEventInfo*> arrExecutingEventList;

 public:
    void SettingEvent();
    bool OnEventTriggerAction(FName triggerName);
    void AddNextExecutiongEventList();
    void ExcuteEventFail();
    void RemoveExecutingEvent(FEventInfo* tartgetEvent);
    void CheckExecuteFunctionTiming(EFunctionExcuteTiming nowTiming, AHotel_Guest* targetGuest);

    void CheckExcuteBasicRule(FName triggerName);
    void AddOutbreakEventList(AHotel_Guest* guest , FName eventName);

    void StartWalkerFireProcess();

    //-----------------------------지역 이벤트---------------------------------------- 
    void Execute_Event_Open205(FEventInfo* eventInfo);//205호 등장
    bool CheckClear_Event_Open205(FEventInfo* eventInfo, FName triggerName);
    void Fail_Event_Open205();

    /*
    void Execute_Event_OpenBasement(FEventInfo* eventInfo);//지하실 등장
    bool CheckClear_Event_OpenBasement(FEventInfo* eventInfo, FName triggerName);

    void Execute_Event_Open305(FEventInfo* eventInfo);//305호 등장
    bool CheckClear_Event_Open305(FEventInfo* eventInfo, FName triggerName);
    */

    //-----------------------------외부 손님---------------------------------------- 
    void Execute_Event_StarePeopleUnderLight(FEventInfo* eventInfo);//가로등 밑의 사람
    void LookingTiemOver_StarePeopleUnderLight();
    bool CheckClear_Event_StarePeopleUnderLight(FEventInfo* eventInfo, FName triggerName);

    void Execute_Event_Invader(FEventInfo* eventInfo); //스테프 온리 문을 안잠그고 다닐시
    void UnlockTiemOver_Invader();
    bool CheckClear_Event_Invader(FEventInfo* eventInfo, FName TriggerName);

    //-----------------------------랜덤 실행 이벤트----------------------------------------

    void Execute_AreaFlickingLight(FEventInfo* eventInfo); // 불 깜빡임
    bool CheckClear_AreaFlickingLight(FEventInfo* eventInfo, FName TriggerName);
    void Execute_Only_MakeDirtyRoom(FEventInfo* eventInfo);    //

    //-----------------------------상황별 추가 이벤트---------------------------------------- 

    void Execute_Event_Complain_RoomDirty(FEventInfo* eventInfo);
    bool CheckClear_Complain_RoomDirty(FEventInfo* eventInfo, FName triggerName);

    void Execute_Event_Request_Reject_CheckIn(FEventInfo* eventInfo);
    bool CheckClear_Request_Reject_CheckIn(FEventInfo* eventInfo, FName triggerName);

    void Execute_Event_Allocate_Room(FEventInfo* eventInfo);
    FString GetAllocateRoomNum();
    bool CheckClear_Allocate_Room(FEventInfo* eventInfo, FName triggerName);

    void Execute_Event_InfinityStair(FEventInfo* eventInfo);         // 무한 계단
    bool CheckClear_Event_InfinityStair(FEventInfo* eventInfo, FName triggerName);

    //-----------------------------체크인 이벤트---------------------------------------- 

    void ExecuteEvent_Guest_LostSignalCCTV(FEventInfo* eventInfo);   //입장시 cctv 끊김
    bool CheckClear_Guest_LostSignalCCTV(FEventInfo* eventInfo, FName triggerName); //

    void ExecuteEvent_Guest_Hanging(FEventInfo* eventInfo);
    bool CheckClear_Guest_Hanging(FEventInfo* eventInfo, FName triggerName);

    void ExecuteEvent_Guest_RoomCCTV(FEventInfo* eventInfo);
    bool CheckClear_Guest_RoomCCTV(FEventInfo* eventInfo, FName triggerName);

    void ExecuteEvent_Guest_InvisibleCamera();

    void ExecuteEvent_PeepingPlayer();


//----------- 세이브 ---------------------
private:
    UHotelSaveGame* HotelSaveGameOption;
    FString MenualSaveName;
    int EnviromentLevel;
    int MenualLevel;
    TArray<int>arrExperiencedEventID;
    TArray<FManualInfo> DefualtMenualText;
    FString FioneerMenualText;

public:
    int GetEnviromentalLevel() { return EnviromentLevel; };
    int GetMenualLevel() { return MenualLevel; };

    void SetFioneerMenaulText(FString temp) { FioneerMenualText = temp; };
    void InitMenualInfo();
    void UpdateDefualtLevelMenual(int EventId);
    FString GetFioneerMenaulText() { return FioneerMenualText; };

    TArray<FManualInfo>* GetMenualInfo() { return &DefualtMenualText; };
    TArray<int>* GetExperiencedEventID() { return &arrExperiencedEventID; };

    void SaveLevelOption(int menual, int Enviroment);
    bool SaveGameManualExternal();
    bool LoadGameManualExternal();
    void LoadGameLevelOption();
    void SaveGameLevelOption();
};