// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Hotel_Walker.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class AHotel_Door;
class UInteractive_Box;
class IInteractable_Object;
struct FInputActionValue;
class UHotel_Manager;
class UPhoneDialUI;
class UMenualUI;
class UDialogueUI;
class AAI_Hotel_Guest_Default;
class AHotel_Phone;
class AHotel_Guest;

UCLASS(Blueprintable)
class BOOTS_ANKLE_HOTEL_API AHotel_Walker : public ACharacter
{
	GENERATED_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EnhancedInput, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultIMC;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EnhancedInput, meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EnhancedInput, meta = (AllowPrivateAccess = "true"))
	UInputAction* IA_Look;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EnhancedInput, meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Interactive;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EnhancedInput, meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Run;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EnhancedInput, meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_QuickAction;

    AHotel_Door* PeepingCamera;
private :
    
    IInteractable_Object* interactObject;

    bool isHit = false;
    bool isRun = false;
    bool isPeeping = false;
    bool isCatchNeck = false;

    bool isEnteringHotel = false;

    FTimerHandle FootStepTimer;
    UFUNCTION()
    void StopMovingSound();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    TSubclassOf<AAI_Hotel_Guest_Default> AIController;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);
    void MoveEnd(const FInputActionValue& Value);

    void RunStart(const FInputActionValue& Value);
    void RunEnd(const FInputActionValue& Value);

    void AutoAction(const FInputActionValue& Value);

    void PlaySound(FName SoundName);
	/** Called for looking input */
	void Look(const FInputActionValue& Value);

    void Interactive(const FInputActionValue& Value);

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UInteractive_Box> InteractiveBoxWidgetClass;
    UInteractive_Box* Interactive_Box;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UUserWidget> CrosshairWidgetClass;
    UUserWidget* Crosshair;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UUserWidget> InteractiveWidgetClass;
    UUserWidget* InteractiveWidget;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UPhoneDialUI> PhoneUiWidgetClass;
    UPhoneDialUI* PhoneUi;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UMenualUI> MenualUiWidgetClass;
    UMenualUI* MenualUi;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UDialogueUI> DialogueUiWidgetClass;
    UDialogueUI* DialogueUi;


    APlayerController* PlayerController;
    UHotel_Manager* Hotel_Manager;
    void SetPeeping(bool peeping, AHotel_Door* peepingEye) {
        isPeeping = peeping;  PeepingCamera = peepingEye;
    };
    void ViewUIInteractiveMessageBox();
    void ViewUIPhoneDial(AHotel_Phone* interactedPhone);
    void ViewUIMenual();
    void ViewUIDialogue(AHotel_Guest * DialogueGuest , AHotel_Phone* Phone = NULL);

    void HideUIName(FName UIName);
    bool IsViewUIName(FName UIName);

    void GetMouseControl();
    void GiveMouseControl(UUserWidget* FocusWidget);
    void TurnOffHighLight();
    void SetCatching(AHotel_Guest* CatchingGuest);

    void StopPeeping();

    void EnterHotel();
    void HideActorFromCamera(AActor * targetActor);
    void ShowActorFromCamera(AActor* targetActor);
    void HangingNeck(FTransform RopeTrans);
    bool IsInteractThisObject(IInteractable_Object* testObject);

    UPROPERTY(EditAnyWhere,BlueprintReadOnly,Category = Sound)
    TMap<FName, USoundBase*> SoundEffectMap;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound)
    UAudioComponent* AudioComp;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    UCameraComponent* FollowCamera;
    USceneCaptureComponent2D* CaptureComp;
};
