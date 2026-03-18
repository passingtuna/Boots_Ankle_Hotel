#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Hotel_Types.generated.h"


class AHotel_Guest_Room;
class AHotel_Guest;
class UEventInfo;

enum EDialogueState
{
    DS_Guest_None, DS_Guest_Default, DS_Guest_TresPass, DS_Guest_CheckIn, DS_Guest_ComplainRoomCondition, DS_Guest_Hanging, DS_Guest_HearingKnock,
    DS_Guest_Complete_RoomClean, DS_Guest_Apologize

    , DS_Manager_Warning, DS_Manager_Fire, DS_Manager_Request_Reject_CheckIn, DS_Manager_Allocate_Room_Guest
    , DS_Security

    , DS_Impostor_Request_Reject_CheckIn
};

struct FGuestDialogueData
{
    FString DialogueTitle           ="";
    EDialogueState DialogueState    = DS_Guest_None;
    int DialogueIndex               = 0;
    FTimerHandle DialogueTimer;
};

USTRUCT(BlueprintType)
struct FDialogueChoice                          //선택지 버튼 구조체
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;                               //버튼 라벨에 보여줄 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int NextLineId = -1;                          //버튼 클릭시 이어줄 다음 노드의 인덱스
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FunctionName;                         //버튼 클릭시 실행할 함수 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int FunctionParameter = 0;   //위에서 실행될 함수에 전달할 값

};

USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int NodeID = 0;                                 //해당 노드의 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Line;                               //대화 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isPausable = true;                     //대화 일시정지가 가능한지 여부 여부에 따라 선택지 버튼 1개 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isDisconnectPhone = false;             //미구현

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueChoice> ChoicesButton;  //선택지 버튼 배열 UI크기상 최대 10개까지만 제한

    UPROPERTY()
    FVector2D NodePosition = FVector2D(0,0);
};

USTRUCT()
struct FRoomInfo
{
    GENERATED_BODY()
    UPROPERTY()
    TObjectPtr<AHotel_Guest_Room> Room;  
    UPROPERTY()
    TObjectPtr<AHotel_Guest> RoomGuest;
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
    FET_Assignment, FET_Init, FET_CheckIn, FET_EnterRoom, FET_CheckOut, FET_WalkerEnterCounter
};

enum EGameEndReason
{
    GER_NotYet, GER_Dead, GER_Fired, GER_Clear
};

UENUM(BlueprintType)
enum class EHotelEventId : int32
{
    None = 0,
    Invader = 1,
    GuestRoomCCTV = 3,
    GuestHanging = 5,
    Open205 = 6,
    StareUnderLight = 7,
    GuestLostSignalCCTV = 8,
    GuestInvisibleCamera = 9,
};

UENUM(BlueprintType)
enum class EHotelOutbreakEventId : uint8
{
    None = 0,
    ComplainRoomDirty,
    RequestRejectCheckIn,
    AllocateRoomGuest,
    MakeDirtyRoom,
    ImpostorRequestRejectCheckIn,
};

struct FExcuteFunctionInfo
{
    int EventID;
    EFunctionExcuteTiming ExecuteTiming = FET_Init;
    bool isEventOnlyOnce = false;
    bool isExcutingOnlyEvent = false;//실행만하고 따로 트리거 체크를 하지않아 대기 리스트에 넣지 않음
    TFunction<void(UEventInfo* eventInfo)> ExecuteFunction;
    TFunction<bool(UEventInfo* eventInfo, FName trigger)> CheckClearFunction;
};

struct FEventInfo
{
    UPROPERTY()
    TObjectPtr<AHotel_Guest> EventGuest;
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
    int32 Category = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EventID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText MenualText;
};

