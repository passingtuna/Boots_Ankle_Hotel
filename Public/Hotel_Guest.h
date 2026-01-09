// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Hotel_Types.h"
#include "Interactable_Object.h"
#include "Hotel_Guest.generated.h"

class UHotel_Manager;
class AHotel_Walker;
class AHotel_Door;
class AAI_Hotel_Guest_Default;
class UDialogueDataAsset;
class USphereComponent;
class AHotel_CCTV_Camera;
class UGuestAnimInstance;
class UGuestFaceAnimInstance;


UCLASS(Blueprintable)
class BOOTS_ANKLE_HOTEL_API AHotel_Guest : public ACharacter , public IInteractable_Object
{
	GENERATED_BODY()

private:
	// Sets default values for this character's properties

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    bool IsLookingPlayer;
public:

    AHotel_Guest();
    AActor* AutoDoor;
    void SetAutoDoor(AActor* Door) { AutoDoor = Door; };

    bool IsInteractable = true;

    bool IsLookingFace;
    bool IsHanging;
    bool IsCheckTrigger;

    bool IsWierdHanging;
    bool IsWierdFaceLook;
    bool IsWierdStareUnderLight;

    bool IsReservationGuest;
    bool bWasLookingAtGuest;
    FString InteractMassage;
    FString ObjectName;
    FString GuestName;
    TMap<FName, TFunction<void()>> availableAction;
    AAI_Hotel_Guest_Default* AIController;
    AHotel_Walker* Hotel_Walker;
    FName QuickActionName;
    AHotel_Door* WaitingDoor;
    UHotel_Manager* Hotel_Manager;


    TArray< FGuestDialogueData> arrGuestDialogueData;
    UDialogueDataAsset* NowDialogueData;
    int LastGuestDialogueDataIndex;


    int nPatience;



    FTimerHandle ComplainTimer;
    FName CheckInRoomNum;
    //
    bool IsMan;
    bool IsReadyToNeckShaking; 
    bool IsAlreadyNeckShaking;
    bool IsAutoActionDoor;
    bool IsCalledWalker;

	// Called every frame
	virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
    TMap<FName, USoundBase*> SoundEffectMap;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision", meta = (AllowPrivateAccess = "true"))
    USphereComponent* FaceCollision;
    UCapsuleComponent* CapsuleComponent;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound)
    UAudioComponent* AudioComp;

    TArray<USkeletalMeshComponent*> MeshComponents;

    UGuestAnimInstance* BedyAnimInstance;
    UGuestFaceAnimInstance* FaceAnimInstance;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    
    virtual TMap<FName, TFunction<void()>> GetAvailableAction() override { return availableAction; };
    virtual void ExecuteActionByName(FName ActionName) override;
    virtual void SetHighLightInteractive(bool OnOff) override;
    virtual void ExecuteQuickAction()override;
    virtual FString GetInteractMessage()override;
    virtual FName GetQuickActionName()override;
    void PlaySound(FName SoundKey);
    void StopSound();
    void OpenConversationUI();
    void SetGuestName(FString Name) {GuestName = Name;};
    void UpdateGuestName();
    void DeactivateGuest();

    void SetDialoguePause(int GuestDialogueDataIndex ,int index);
    virtual UDialogueDataAsset* GetDailogueData(EDialogueState DialogueState, bool isCall);
    void SetGuestDialogueDataLast(bool isCalled);

    void EndGuestDialgue(int GuestDialogueDataIndex);
    void AddDialogueDataState(FString title, EDialogueState state ,int Index = 0);
    void EraseDialgueDataState(EDialogueState state);

    void SetGuestDialogueData(UDialogueDataAsset* dialogueData, int DataIndex);

    UFUNCTION()
    void ComplainToManager();
    void SetComplainTimer();
    void UpdateInteractMessage();
    void CorpseRetrieval();
    void SetHangingState(bool state);
    void SetNeckShakingState(bool state);
    void SetLookingCameraStatue(int state , AHotel_CCTV_Camera * camera);
    void SetAutoActionDoor(bool state) { IsAutoActionDoor = state;  };
    bool GetAutoActionDoor() { return IsAutoActionDoor; };
    void CheckOutGuest();
    void ReleaseDoorLock();
    void GuestExit();

    void SetLookingPlayer(bool state);
    void HearingKnockAction();
    void SetLookPlayer();
    void SetLookActor(AActor * targetActor);
    void ArrivedKnockingDoor(AHotel_Door* Door);
    void ReadyToMove();
    void ReadyToRotate();
    void CheckWalkerLookingFace(FVector GazeLocation, FVector PlayerGaze);
    void CatchingPlayer();
    void SetAIController(AAI_Hotel_Guest_Default * controller) {AIController = controller;};
    AAI_Hotel_Guest_Default* GetAIController() { return AIController; };

    bool GetIsLookingFace() { return IsLookingFace; };
    virtual void CallingFailAction();


    virtual bool GetIsInteractive()override { return IsInteractable; };
    virtual void SetIsInteractive(bool on)override { IsInteractable = on; };

};