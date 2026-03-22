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
class AHotel_Light;
class AHotel_Place;
class UHotel_EventBase;
class UHotel_OutbreakEventBase;
class AHotel_Operator;

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
    TObjectPtr<UHotel_EventBase> EventLogic;
    UPROPERTY()
    TObjectPtr<AHotel_Guest> EventGuest;
    FTimerHandle EventTimer;
    bool isNormalGuestEvent = true;
    bool isAreadyExcute = false;
    TArray<FHotelTrigger> CollectedTriggers;
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
    TArray<TObjectPtr<UHotel_EventBase>> EventObjects;

    UPROPERTY()
    TArray<TObjectPtr<UHotel_OutbreakEventBase>> OutbreakEventObjects;

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

    UPROPERTY(EditDefaultsOnly, Category="Guest Spawn")
    int32 MaxGuestsPerRoundHardCap = 26;
    UPROPERTY(EditDefaultsOnly, Category="Guest Spawn")
    int32 GuestsPerSpawnTick = 1;
    UPROPERTY(EditDefaultsOnly, Category="Guest Spawn")
    float GuestSpawnTickIntervalSeconds = 0.05f;

    // EnterHotel/SettingEvent 진입 시점에 스폰을 더 이상 진행하지 않기 위한 락.
    // SettingEvent는 arrWaitingEventList를 기반으로 이름/이벤트를 초기화하므로, 스폰이 동시에 진행되면 데이터 불일치가 날 수 있습니다.
    bool bStopSpawnOnSettingEvent = false;

    /** Initialize() 단계별 분리: 서비스·이벤트 오브젝트·에셋 로드 책임을 나눔 */
    void EnsurePersistenceAndHRInitialized();
    void EnsureRuntimeEventObjectsCreated();
    void LoadPrimaryAssetDialogueAndGuestData();
    void RebuildActiveEventFunctionList();

public:
    void LoadGuestName();
    void SetHotelWalker(AHotel_Walker* Walker) { Hotel_Walker = Walker; };
    void RegisterWalkerAndStartGuestPrep(AHotel_Walker* Walker);
    void HandleWalkerEnterHotelForRoundSetup();
    void ScheduleDelayedGameOver(int32 HRPenalty, const FString& HRReason, EGameEndReason Reason, float DelaySeconds = 3.f);
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

    // SpawnGuest용 "단일 반복 타이머" 핸들 (프레임 분산/스폰 폭주 방지)
    FTimerHandle SpawnGuestLoopTimerHandle;
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
    AHotel_Door* GetStaffDoor() { return StaffDoor; }
    AHotel_Light* GetStreetLight() { return StreetLight; }
    AActor* GetAutoDoor() { return AutoDoor; }
    AHotel_Place* GetPlaceByName(FName Name);
    AHotel_Operator* GetDepartmentOperator() { return Department_Operator; }

    class AHotel_CCTV* GetCCTV() { return Hotel_CCTV; }
    class UHotel_StaticMesh* GetHotelMeshByName(FName Name);
    FRoomInfo* FindRoomInfoByName(FName RoomKey);
    const TMap<FName, FRoomInfo>& GetRegisteredRooms() const { return mapRegistedRoom; }
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
     TArray<TObjectPtr<UHotel_EventBase>> arrEventFuntionList;
     TArray <TObjectPtr<UEventInfo>> arrOutbreakEventList;
     TArray <TObjectPtr<UEventInfo>> arrWaitingEventList;

     TArray <TObjectPtr<UEventInfo>> arrExecutingEventList;
     bool ProcessCallException(AHotel_Phone* CalledPhone, FName NowCallingPhoneNum);

 public:
    void SettingEvent();
    bool OnEventTriggerAction(const FHotelTrigger& trigger);
    void AddNextExecutiongEventList();
    void ExcuteEventFail();
    void RemoveExecutingEvent(UEventInfo* tartgetEvent);
    void CheckExecuteFunctionTiming(EFunctionExcuteTiming nowTiming, AHotel_Guest* targetGuest);

    void CheckExcuteBasicRule(const FHotelTrigger& Trigger);
    void AddOutbreakEventList(AHotel_Guest* guest , EHotelOutbreakEventId eventId);


    FString GetAllocateRoomNum();
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