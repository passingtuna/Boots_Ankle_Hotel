// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Hotel_Types.h"
#include "Hotel_Manager.generated.h"

class AHotel_Phone;
class AHotel_Walker;
class AHotel_Guest;
class AHotel_Guest_Room;
class AHotel_Place;
class AHotel_Clock;
class AHotel_KeyTray;
class AHotel_Switch;
class AHotel_Operator;
class AHotel_CCTV;
class AHotel_Light;
class AHotel_StaticMesh;
class UConversationUI;
class UHotelSaveGame;
class ALevel_Manager;
class UDialogueDataAsset;
class UGuestDataAsset;
class AHotel_Door;
class UHotel_StaticMesh;
class AAI_Hotel_Guest_Default;
class UHotel_PersistenceService;
class UHotel_HRService;

static const TArray<FString> CodeWords =
{
    TEXT("앞코"),TEXT("뒷굽"),TEXT("부츠끈"),TEXT("혀"),
    TEXT("목통"),TEXT("밑창"),TEXT("덧창"),TEXT("깔창"),
    TEXT("안창"),TEXT("장식못")
};

UCLASS()
class UEventInfo : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY()
    TObjectPtr<AHotel_Guest> EventGuest;
    FTimerHandle EventTimer;
    bool isNormalGuestEvent = true;
    bool isAreadyExcute = false;
    TArray<FName> CollectedTriggers;
    FExcuteFunctionInfo FunctionInfo;
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
    
    TArray<AHotel_Guest*> arrHotelGuest;

    AHotel_KeyTray* KeyTray;
    AHotel_Walker* Hotel_Walker;
    AHotel_Operator* Department_Operator;
    int CodeWord;
    bool IsAlreadyInit;
    ALevel_Manager* Level_Manager;

    bool IsGameEndPhase;

    UPROPERTY()
    TObjectPtr<UHotel_PersistenceService> PersistenceService;

    UPROPERTY()
    TObjectPtr<UHotel_HRService> HRService;

    UPROPERTY()
    TMap<FName, AHotel_Phone*> mapRegistedPhone;
    UPROPERTY()
    TMap<FName, AHotel_Place*> mapRegistedPlace;
    UPROPERTY()
    TMap<FName, UDialogueDataAsset*> DialogueMap;
    TMap<FName, FRoomInfo> mapRegistedRoom;
    UPROPERTY()
    TMap<FName, UHotel_StaticMesh*> mapHotelMesh;
    UPROPERTY()
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
    FTimerHandle GameEndTimer;
    FName WalkerNowLocation;
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

    TArray <FHRRecord> GetHRRecord();
    int GetHRScore();

    bool GetWalkerFired();
    FString GetReservationGuestName();
 //-----------------------이벤트----------------------------------
 private:
     int nNowExcutingEvent;
     TArray <FExcuteFunctionInfo> arrEventFuntionList;
     TMap <FName, FExcuteFunctionInfo> mapOutbreakEventFuntion;
     TArray <TObjectPtr<UEventInfo>> arrOutbreakEventList;
     TArray <TObjectPtr<UEventInfo>> arrWaitingEventList;

     TArray <TObjectPtr<UEventInfo>> arrExecutingEventList;
     bool ProcessCallException(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum);

 public:
    void SettingEvent();
    bool OnEventTriggerAction(FName triggerName);
    void AddNextExecutiongEventList();
    void ExcuteEventFail();
    void RemoveExecutingEvent(UEventInfo* tartgetEvent);
    void CheckExecuteFunctionTiming(EFunctionExcuteTiming nowTiming, AHotel_Guest* targetGuest);

    void CheckExcuteBasicRule(FName triggerName);
    void AddOutbreakEventList(AHotel_Guest* guest , FName eventName);


    //-----------------------------지역 이벤트---------------------------------------- 
    void Execute_Event_Open205(UEventInfo*  eventInfo);//205호 등장
    bool CheckClear_Event_Open205(UEventInfo*  eventInfo, FName triggerName);
    void Fail_Event_Open205();

    /*
    void Execute_Event_OpenBasement(FEventInfo* eventInfo);//지하실 등장
    bool CheckClear_Event_OpenBasement(FEventInfo* eventInfo, FName triggerName);

    void Execute_Event_Open305(FEventInfo* eventInfo);//305호 등장
    bool CheckClear_Event_Open305(FEventInfo* eventInfo, FName triggerName);
    */

    //-----------------------------외부 손님---------------------------------------- 
    void Execute_Event_StarePeopleUnderLight(UEventInfo*  eventInfo);//가로등 밑의 사람
    void LookingTiemOver_StarePeopleUnderLight();
    bool CheckClear_Event_StarePeopleUnderLight(UEventInfo*  eventInfo, FName triggerName);

    void Execute_Event_Invader(UEventInfo*  eventInfo); //스테프 온리 문을 안잠그고 다닐시
    void UnlockTiemOver_Invader();
    bool CheckClear_Event_Invader(UEventInfo*  eventInfo, FName TriggerName);

    //-----------------------------랜덤 실행 이벤트----------------------------------------

    void Execute_AreaFlickingLight(UEventInfo*  eventInfo); // 불 깜빡임
    bool CheckClear_AreaFlickingLight(UEventInfo*  eventInfo, FName TriggerName);
    void Execute_Only_MakeDirtyRoom(UEventInfo*  eventInfo);    //
    void Execute_Only_Imposter_Request_Reject_Check(UEventInfo*  eventInfo);    //

    //-----------------------------상황별 추가 이벤트---------------------------------------- 

    void Execute_Event_Complain_RoomDirty(UEventInfo*  eventInfo);
    bool CheckClear_Complain_RoomDirty(UEventInfo*  eventInfo, FName triggerName);

    void Execute_Event_Request_Reject_CheckIn(UEventInfo*  eventInfo);
    bool CheckClear_Request_Reject_CheckIn(UEventInfo*  eventInfo, FName triggerName);

    void Execute_Event_Allocate_Room(UEventInfo*  eventInfo);
    FString GetAllocateRoomNum();
    bool CheckClear_Allocate_Room(UEventInfo*  eventInfo, FName triggerName);

    void Execute_Event_InfinityStair(UEventInfo*  eventInfo);         // 무한 계단
    bool CheckClear_Event_InfinityStair(UEventInfo*  eventInfo, FName triggerName);

    //-----------------------------체크인 이벤트---------------------------------------- 

    void ExecuteEvent_Guest_LostSignalCCTV(UEventInfo*  eventInfo);   //입장시 cctv 끊김
    bool CheckClear_Guest_LostSignalCCTV(UEventInfo*  eventInfo, FName triggerName); //

    void ExecuteEvent_Guest_Hanging(UEventInfo*  eventInfo);
    bool CheckClear_Guest_Hanging(UEventInfo*  eventInfo, FName triggerName);

    void ExecuteEvent_Guest_RoomCCTV(UEventInfo*  eventInfo);
    bool CheckClear_Guest_RoomCCTV(UEventInfo*  eventInfo, FName triggerName);

    void ExecuteEvent_Guest_InvisibleCamera(UEventInfo*  eventInfo);
    bool CheckClear_Guest_InvisibleCamera(UEventInfo*  eventInfo, FName triggerName); //

    void ExecuteEvent_PeepingPlayer();


//----------- 세이브/옵션 ---------------------
public:
    int GetEnviromentalLevel();
    int GetMenualLevel();

    void SetFioneerMenaulText(FString temp);
    void InitMenualInfo();
    void UpdateDefualtLevelMenual(int EventId);
    FString GetFioneerMenaulText();

    TArray<FManualInfo>* GetMenualInfo();
    TArray<int>* GetExperiencedEventID();

    void SaveLevelOption(int menual, int Enviroment);
    bool SaveGameManualExternal();
    bool LoadGameManualExternal();
    void LoadGameLevelOption();
    void SaveGameLevelOption();
};