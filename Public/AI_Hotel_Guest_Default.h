// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AI_Hotel_Guest_Default.generated.h"

/**
 * 
 */
class AHotel_Guest_Room;
class AHotel_Guest;
class UHotel_Manager;

enum EArriveState
{
    AS_None, AS_EndMoveCounter , AS_EndMoveRoom, AS_EndHearingKnock ,AS_LockingDoor,AS_OutSide
};

UCLASS()
class BOOTS_ANKLE_HOTEL_API AAI_Hotel_Guest_Default : public AAIController
{
    GENERATED_BODY()
    virtual void Tick(float DeltaTime) override;
private:
    bool isComplained;
    int nPatience;
    FTimerHandle Timer_AI;
    FTimerHandle Timer_Patience;
public:
    void BeginPlay()override;
    void MoveToTargetLocation(FVector TargetLocation);
    UPROPERTY(EditAnywhere, Category = "AI")
    float AcceptableRadius = 50.0f;
    int MoveSuccessState;

    UFUNCTION()
    void OnMoveCompletedCallback(FAIRequestID RequestID, const EPathFollowingResult::Type Result);

    void AssigningGuestRoom(AHotel_Guest_Room* GuestRoom);
    void InitAIController(AHotel_Guest* Guest);
    void GoToCounter();
    void GoToRoom();
    void GoToStaffPlace();
    void GoToOutside();
    void GoToRoomDoor();
    AHotel_Guest_Room* AssignedGuestRoom;
    AHotel_Guest* Hotel_Guest;
    UHotel_Manager* Hotel_Manager;
    void CallingFail();
    void CheckingRoomCondition();
    void HearingKnock();
    void CallingWalker();
    void DecreasePatienceCount(bool Looping = false);
    int GetPatienceCount() {return nPatience;};
    void RingingBell();
    void StopPatienceTimer();
    void StopAITimer();
    void CloseRoomDoor(float time);

    bool IsMovingToTarget();
};
