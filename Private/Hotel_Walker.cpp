// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Walker.h"
#include "Engine/LocalPlayer.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Interactive_Box.h"
#include "Hotel_Door.h"
#include "Hotel_Manager.h"
#include "Hotel_Object.h"
#include "Hotel_Guest.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SphereComponent.h"
#include "Level_Manager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Interactable_Object.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Hotel_Menual.h"
#include "DialogueDataAsset.h"
#include "Hotel_Types.h"

AHotel_Walker::AHotel_Walker()
{
	LocomotionComp = CreateDefaultSubobject<UHotel_WalkerLocomotionComponent>(TEXT("LocomotionComp"));
	InteractionComp = CreateDefaultSubobject<UHotel_WalkerInteractionComponent>(TEXT("InteractionComp"));
	UIComp = CreateDefaultSubobject<UHotel_WalkerUIComponent>(TEXT("UIComp"));
	PeepingComp = CreateDefaultSubobject<UHotel_WalkerPeepingComponent>(TEXT("PeepingComp"));
	EntranceIntroComp = CreateDefaultSubobject<UHotel_WalkerEntranceIntroComponent>(TEXT("EntranceIntroComp"));
}

void AHotel_Walker::BeginPlay()
{
	Super::BeginPlay();
    // 위젯 생성/입력 기반 처리에 필요하므로 BeginPlay에서 먼저 확보합니다.
    PlayerController = Cast<APlayerController>(GetController());

    if (LocomotionComp)
    {
        LocomotionComp->Initialize(this);
    }
    FollowCamera = FindComponentByClass<UCameraComponent>();
    CaptureComp = FindComponentByClass<USceneCaptureComponent2D>();
    if (UIComp)
    {
        UIComp->Initialize(this, PlayerController);
    }
    if (InteractionComp)
    {
        InteractionComp->Initialize(this);
    }
    if (PeepingComp)
    {
        PeepingComp->Initialize(this);
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = PlayerController
        ? ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())
        : nullptr;
    if (Subsystem)
    {
        Subsystem->AddMappingContext(DefaultIMC, 0);
    }

    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    if (Hotel_Manager)
    {
        Hotel_Manager->RegisterWalkerAndStartGuestPrep(this);
    }
    //EnterHotel();
    //메인 메뉴
    AddControllerPitchInput(-10); //메인메뉴 띄우기 뷰 조정
}

// Called every frame
void AHotel_Walker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (EntranceIntroComp && EntranceIntroComp->TickIntro(DeltaTime, this, LocomotionComp, UIComp))
    {
        return;
    }

    if ((PeepingComp && PeepingComp->IsPeeping()) || isCatchNeck || (PlayerController && PlayerController->bShowMouseCursor))
    {
        if (InteractionComp)
        {
            InteractionComp->ClearInteractionFocusForBlockedState();
        }
        return;
    }

    if (FollowCamera && InteractionComp)
    {
        InteractionComp->TickInteraction(DeltaTime, FollowCamera);
    }
}

void AHotel_Walker::Interactive(const FInputActionValue& Value)
{
    if (isCatchNeck) return;
    if (!PeepingComp || !PeepingComp->IsPeeping())
    {
        IInteractable_Object* Target = InteractionComp ? InteractionComp->GetCurrentInteractable() : nullptr;
        if (Target && UIComp && UIComp->GetInteractiveBox())
        {
            if (Target->GetAvailableAction().IsEmpty()) return;//오브젝트가 할수있는 행동이 없다면 리턴
            UIComp->GiveMouseControlTo(UIComp->GetInteractiveBox());
            TurnOffHighLight();
            UIComp->HideByLegacyName(FName(TEXT("InteractMessage")));
            UIComp->GetInteractiveBox()->ShowActionButtons(Target);
        }
    }
}

void AHotel_Walker::AutoAction(const FInputActionValue& Value)
{
    if (isCatchNeck) return;
    if (!PeepingComp || !PeepingComp->IsPeeping())
    {
        if (IInteractable_Object* Target = InteractionComp ? InteractionComp->GetCurrentInteractable() : nullptr)
        {
            Target->ExecuteQuickAction();
        }
    }
}


// Called to bind functionality to input
void AHotel_Walker::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
		if (EnhancedInputComponent)
		{
			EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AHotel_Walker::Move);
            EnhancedInputComponent->BindAction(IA_Move, ETriggerEvent::Completed, this, &AHotel_Walker::MoveEnd);

			EnhancedInputComponent->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AHotel_Walker::Look);

            EnhancedInputComponent->BindAction(IA_Run, ETriggerEvent::Started, this, &AHotel_Walker::RunStart);
            EnhancedInputComponent->BindAction(IA_Run, ETriggerEvent::Completed, this, &AHotel_Walker::RunEnd);

            EnhancedInputComponent->BindAction(IA_Interactive, ETriggerEvent::Started, this, &AHotel_Walker::Interactive);

            EnhancedInputComponent->BindAction(IA_QuickAction, ETriggerEvent::Started, this, &AHotel_Walker::AutoAction);
		}
	}
}


void AHotel_Walker::MoveEnd(const FInputActionValue& Value)
{
    if (LocomotionComp)
    {
        LocomotionComp->ProcessMoveEnd(Value);
    }
}

void AHotel_Walker::RunStart(const FInputActionValue& Value)
{
    if (LocomotionComp)
    {
        LocomotionComp->ProcessRunStarted(Value);
    }
}

void AHotel_Walker::RunEnd(const FInputActionValue& Value)
{
    if (LocomotionComp)
    {
        LocomotionComp->ProcessRunEnded(Value);
    }
}

void AHotel_Walker::Move(const FInputActionValue& Value)
{
    if (isCatchNeck) return;
    if (PlayerController && PlayerController->bShowMouseCursor) return;

    if (PeepingComp && PeepingComp->IsPeeping())
    {
        StopPeeping();
        return;
    }

    if (LocomotionComp)
    {
        LocomotionComp->ProcessMoveInput(Value);
    }
}

void AHotel_Walker::SetPeeping(bool peeping, AHotel_Door* peepingEye)
{
    if (PeepingComp)
    {
        PeepingComp->SetPeepingState(peeping, peepingEye);
    }
}

AHotel_Door* AHotel_Walker::GetActivePeepingDoor() const
{
    return PeepingComp ? PeepingComp->GetActivePeepingDoor() : nullptr;
}

void AHotel_Walker::StopPeeping()
{
    if (PeepingComp)
    {
        PeepingComp->StopPeeping();
    }
}

void AHotel_Walker::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (isCatchNeck) return;
    if (PeepingComp && PeepingComp->IsPeeping())
    {
        PeepingComp->ApplyPeepingLook(LookAxisVector);
    }
    else
    {
        if (Controller != nullptr)
        {
            AddControllerYawInput(LookAxisVector.X);
            AddControllerPitchInput(LookAxisVector.Y);
        }
    }
}


void AHotel_Walker::GiveMouseControl(UUserWidget* FocusWidget)
{
    if (UIComp)
    {
        UIComp->GiveMouseControlTo(FocusWidget);
    }
}
void AHotel_Walker::TurnOffHighLight()
{
    if (InteractionComp)
    {
        InteractionComp->TurnOffHighlightOnly();
    }
}
void AHotel_Walker::GetMouseControl()
{
    if (UIComp)
    {
        UIComp->ReleaseMouseToGame();
    }
}

void AHotel_Walker::PlaySound(FName SoundName)
{
    USoundBase* sb_Sound;

    if (SoundEffectMap.Contains(SoundName))
    {
        sb_Sound = *SoundEffectMap.Find(SoundName);
        if (AudioComp->GetSound() != sb_Sound)
        {
            AudioComp->SetSound(sb_Sound);
            AudioComp->Play();
        }
    }
}

void AHotel_Walker::ViewUIInteractiveMessageBox()
{
    if (UIComp && InteractionComp)
    {
        UIComp->UpdateInteractionPrompt(InteractionComp->GetCurrentInteractable());
    }
}

void AHotel_Walker::HideInteractionPromptWidget()
{
    if (UIComp)
    {
        UIComp->HideInteractionPrompt();
    }
}
void AHotel_Walker::ViewUIPhoneDial(AHotel_Phone* interactedPhone)
{
    if (UIComp)
    {
        UIComp->ViewPhoneDial(this, interactedPhone);
    }
}

void AHotel_Walker::ViewUIMenual()
{
    if (UIComp)
    {
        UIComp->ViewManual(this);
    }
}

void AHotel_Walker::ViewUIDialogue(AHotel_Guest* DialogueGuest, AHotel_Phone * Phone)
{
    if (UIComp)
    {
        UIComp->ViewDialogue(this, DialogueGuest, Phone);
    }
}
void AHotel_Walker::HideUIName(FName UIName)
{
    if (UIComp)
    {
        UIComp->HideByLegacyName(UIName);
    }
}

bool AHotel_Walker::IsViewUIName(FName UIName)
{
    return UIComp && UIComp->IsLegacyNameVisible(UIName);
}

bool AHotel_Walker::IsInteractThisObject(IInteractable_Object* testObject)
{
    return InteractionComp && InteractionComp->IsTargeting(testObject);
}


void AHotel_Walker::ApplyRestraintBaseState()
{
    isCatchNeck = true;
    if (!bUseControllerRotationYaw)
    {
        if (UCharacterMovementComponent* Move = GetCharacterMovement())
        {
            Move->bOrientRotationToMovement = false;
        }
        bUseControllerRotationYaw = false;
    }
    if (UCapsuleComponent* Cap = GetCapsuleComponent())
    {
        Cap->SetCollisionProfileName(TEXT("NoCollision"));
    }
}

void AHotel_Walker::ScheduleDelayedFailViaManager(const FString& HRReason)
{
    if (Hotel_Manager)
    {
        Hotel_Manager->ScheduleDelayedGameOver(100, HRReason, EGameEndReason::GER_Dead, 3.f);
    }
}

void AHotel_Walker::SetCatching(AHotel_Guest* CatchingGuest)
{
    if (isCatchNeck || !CatchingGuest)
    {
        return;
    }

    ApplyRestraintBaseState();

    const FVector MyLocation = GetActorLocation();
    const FVector TargetLocation = CatchingGuest->GetActorLocation();
    const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLocation, TargetLocation);
    SetActorRotation(FRotator(0, LookAtRot.Yaw, 0));
    if (PlayerController)
    {
        PlayerController->SetControlRotation(LookAtRot);
    }

    ScheduleDelayedFailViaManager(TEXT("사망으로 인한 근무지 이탈"));
}

void AHotel_Walker::EnterHotel()
{
    if (EntranceIntroComp)
    {
        EntranceIntroComp->StartIntro();
    }
    if (Hotel_Manager)
    {
        Hotel_Manager->HandleWalkerEnterHotelForRoundSetup();
    }
    if (UIComp)
    {
        UIComp->NotifyGameStartForManual();
    }
}


void AHotel_Walker::HideActorFromCamera(AActor* targetActor)
{
    CaptureComp->HiddenActors.Add(targetActor);
}
void AHotel_Walker::ShowActorFromCamera(AActor* targetActor)
{
    CaptureComp->HiddenActors.Remove(targetActor);
}

void AHotel_Walker::HangingNeck(FTransform RopeTrans)
{
    ApplyRestraintBaseState();

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->GravityScale = 0.0f;
        Move->Velocity.Z = 0.0f;
    }

    const FVector RelativeOffset(0, -5, -80);
    const FVector WorldPos = RopeTrans.TransformPosition(RelativeOffset);
    FRotator EditedRot = RopeTrans.Rotator();
    EditedRot.Pitch -= 20;
    SetActorRotation(FRotator(0, EditedRot.Yaw, 0));
    if (PlayerController)
    {
        PlayerController->SetControlRotation(EditedRot);
    }
    TeleportTo(WorldPos, RopeTrans.Rotator());

    ScheduleDelayedFailViaManager(TEXT("근무중 실종"));
}
