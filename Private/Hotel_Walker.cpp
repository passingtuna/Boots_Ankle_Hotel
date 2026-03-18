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
#include "Components/TextBlock.h"
#include "Components/SphereComponent.h"
#include "Level_Manager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Interactable_Object.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "PhoneDialUI.h"
#include "MenualUI.h"
#include "DialogueUI.h"
#include "Hotel_Menual.h"
#include "DialogueDataAsset.h"

// Called when the game starts or when spawned

void AHotel_Walker::BeginPlay()
{
	Super::BeginPlay();
    //AdudioComp
    isRun = false;
    FollowCamera = FindComponentByClass<UCameraComponent>();
    CaptureComp = FindComponentByClass<USceneCaptureComponent2D>();
    if (InteractiveBoxWidgetClass)
    {
        Interactive_Box = CreateWidget<UInteractive_Box>(PlayerController, InteractiveBoxWidgetClass);
        if (Interactive_Box)
        {
            Interactive_Box->AddToViewport();
            Interactive_Box->SetVisibility(ESlateVisibility::Hidden);
            Interactive_Box->SetHotelWalker(this);
        }
    }

    if (CrosshairWidgetClass)
    {
        Crosshair = CreateWidget<UUserWidget>(PlayerController, CrosshairWidgetClass);
        if (Crosshair)
        {
            Crosshair->AddToViewport();
            Crosshair->SetVisibility(ESlateVisibility::Visible);
        }
    }

    if (InteractiveWidgetClass)
    {
        InteractiveWidget = CreateWidget<UUserWidget>(PlayerController, InteractiveWidgetClass);
        if (InteractiveWidget)
        {
            InteractiveWidget->AddToViewport();
            InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (PhoneUiWidgetClass)
    {
        PhoneUi = CreateWidget<UPhoneDialUI>(PlayerController, PhoneUiWidgetClass);
        if (PhoneUi)
        {
            PhoneUi->AddToViewport();
            PhoneUi->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (MenualUiWidgetClass)
    {
        MenualUi = CreateWidget<UMenualUI>(PlayerController, MenualUiWidgetClass);
        if (MenualUi)
        {
            MenualUi->AddToViewport(10);
            MenualUi->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (DialogueUiWidgetClass)
    {
        DialogueUi = CreateWidget<UDialogueUI>(PlayerController, DialogueUiWidgetClass);
        if (DialogueUi)
        {
            DialogueUi->AddToViewport();
            DialogueUi->SetVisibility(ESlateVisibility::Hidden);
        }
    }


    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
    if (Subsystem)
    {
        Subsystem->AddMappingContext(DefaultIMC, 0);
    }

    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();

    Hotel_Manager->SetHotelWalker(this);
    isPeeping = false;
    Hotel_Manager->SpawnGuest(); //워커까지 준비완료후 손님 준비
    //EnterHotel();
    //메인 메뉴
    AddControllerPitchInput(-10); //메인메뉴 띄우기 뷰 조정
}

// Called every frame
void AHotel_Walker::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (isEnteringHotel)
    {
        FRotator MyRotator = Controller->GetControlRotation();
        if (MyRotator.Pitch > 5)
        {
            AddControllerPitchInput(0.5f);
            if (MyRotator.Pitch > 30)
            {
                MyRotator.Pitch = 5;
                Controller->SetControlRotation(MyRotator);
            }
        }
        else if (GetActorLocation().Y < 1400)
        {
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);
            FVector2D MovementVector = FVector2D(0, 0.6f);
            const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            AddMovementInput(ForwardDirection, MovementVector.Y);
            PlaySound("Walk");
        }
        else
        {
            GetWorld()->GetTimerManager().SetTimer(FootStepTimer, this, &AHotel_Walker::StopMovingSound, 0.2f, false);
            GetMouseControl();
            isEnteringHotel = false;
        }
        return;
    }
    
    if (isPeeping || PlayerController->bShowMouseCursor || isCatchNeck)
    {
        if (interactObject)
        {
            interactObject->SetHighLightInteractive(false);
            InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
        }
        return;
    }

    if (FollowCamera)
    {
        // 상호작용 라인트레이스는 너무 자주 할 필요가 없으므로 주기를 둔다.
        InteractionTraceElapsed += DeltaTime;
        if (InteractionTraceElapsed < 0.05f)
        {
            return;
        }
        InteractionTraceElapsed = 0.0f;

        FVector Start = FollowCamera->GetComponentLocation();
        FVector End = Start + FollowCamera->GetForwardVector() * 200.f;

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        //라인 트레이스
        isHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

        if (interactObject)
        {
            interactObject->SetHighLightInteractive(false);
            InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
            interactObject = NULL;
        }
        if (isHit)
        {
            AActor* HitActor = Hit.GetActor();

            //액터가 UInteractable_Object 인터페이스를 상속받았는지 확인
            if (HitActor)
            {
                if (HitActor->GetClass()->ImplementsInterface(UInteractable_Object::StaticClass()))
                {
                    interactObject = Cast<IInteractable_Object>(HitActor);
                    interactObject->SetHighLightInteractive(true);
                    ViewUIInteractiveMessageBox();
                    if (AHotel_Guest*temp = Cast<AHotel_Guest>(HitActor))
                    {
                        temp->CheckWalkerLookingFace(Start, End);
                    }
                }
            }
            else
            {
                if (interactObject)
                {
                    interactObject->SetHighLightInteractive(false);
                    InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
                    interactObject = NULL;
                }
            }
        }
    }
}

void AHotel_Walker::Interactive(const FInputActionValue& Value)
{
    if (isCatchNeck) return;
    if (!isPeeping)
    {
        if (interactObject && Interactive_Box)
        {
            if (interactObject->GetAvailableAction().IsEmpty()) return;//오브젝트가 할수있는 행동이 없다면 리턴
            GiveMouseControl(Interactive_Box);
            TurnOffHighLight();
            HideUIName("InteractMessage");
            Interactive_Box->ShowActionButtons(interactObject);
        }
    }
}

void AHotel_Walker::AutoAction(const FInputActionValue& Value)
{
    if (isCatchNeck) return;
    if (!isPeeping)
    {
        if (interactObject)
        {
            interactObject->ExecuteQuickAction();
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
    GetWorld()->GetTimerManager().SetTimer(FootStepTimer,this, &AHotel_Walker::StopMovingSound ,0.2f,false);
}

void AHotel_Walker::RunStart(const FInputActionValue& Value)
{
    isRun = true;
}

void AHotel_Walker::RunEnd(const FInputActionValue& Value)
{
    isRun = false;
}
void AHotel_Walker::StopMovingSound()
{
    AudioComp->Stop();
    AudioComp->SetSound(NULL);
}

void AHotel_Walker::Move(const FInputActionValue& Value)
{
    if (isCatchNeck) return;
    if (PlayerController->bShowMouseCursor) return;
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (!isRun)
    {
        PlaySound("Walk");
        MovementVector *= 0.6;
    }
    else
    {
        PlaySound("Run");
    }

    if (isPeeping)
    {
        StopPeeping();
    }
    else
    {
        if (Controller != nullptr)
        {
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

            const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

            AddMovementInput(ForwardDirection, MovementVector.Y);
            AddMovementInput(RightDirection,   MovementVector.X);
        }
    }
}

void AHotel_Walker::StopPeeping()
{
    if (!isPeeping) return;
    PlayerController->SetViewTargetWithBlend(this);
    isPeeping = false;
    PeepingCamera->PeepingEnd();
    PeepingCamera = NULL;
}

void AHotel_Walker::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (isCatchNeck) return;
    if (isPeeping)
    {
        if(PeepingCamera)
        {
            PeepingCamera->MovePeepingEye(LookAxisVector);
        }
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
    if (PlayerController)
    {
        PlayerController->bShowMouseCursor = true;
        FInputModeUIOnly InputModeData;
        if (FocusWidget)
        {
            InputModeData.SetWidgetToFocus(FocusWidget->TakeWidget()); // 포커스를 UI로
        }
        InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 자유롭게
        PlayerController->SetInputMode(InputModeData);
        Crosshair->SetVisibility(ESlateVisibility::Hidden);
    }
}
void AHotel_Walker::TurnOffHighLight()
{
    if(interactObject)interactObject->SetHighLightInteractive(false);
}
void AHotel_Walker::GetMouseControl()
{
    if (PlayerController)
    {
        FInputModeGameOnly InputModeData;
        PlayerController->SetInputMode(InputModeData);
        PlayerController->bShowMouseCursor = false;
    }
    Crosshair->SetVisibility(ESlateVisibility::Visible);
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
    if (InteractiveWidget)
    {
        UTextBlock* Text = Cast<UTextBlock>(InteractiveWidget->GetWidgetFromName(TEXT("InteractText")));
        Text->SetText(FText::FromString(interactObject->GetInteractMessage()));
        InteractiveWidget->SetVisibility(ESlateVisibility::Visible);
    }
}
void AHotel_Walker::ViewUIPhoneDial(AHotel_Phone* interactedPhone)
{
    TurnOffHighLight();
    if (PhoneUi)
    {
        GiveMouseControl(PhoneUi);
        PhoneUi->SetVisibility(ESlateVisibility::Visible);
        PhoneUi->SetPhoneObject(interactedPhone);
    }
}

void AHotel_Walker::ViewUIMenual()
{
    TurnOffHighLight();
    if (MenualUi)
    {
        GiveMouseControl(MenualUi);
        MenualUi->InitMenual(this);
        MenualUi->SetVisibility(ESlateVisibility::Visible);
    }
}

void AHotel_Walker::ViewUIDialogue(AHotel_Guest* DialogueGuest, AHotel_Phone * Phone)
{
    TurnOffHighLight();
    HideUIName("InteractMessage");
    if (DialogueUi)
    {
        GiveMouseControl(DialogueUi);
        DialogueUi->SetDialogueGuest(DialogueGuest , Phone);
        DialogueUi->SetHotelWalker(this);
        DialogueUi->SetVisibility(ESlateVisibility::Visible);
    }
}
void AHotel_Walker::HideUIName(FName UIName)
{
    if (UIName == "InteractBox")
    {
        Interactive_Box->SetVisibility(ESlateVisibility::Hidden);
    }
    else if (UIName == "InteractMessage")
    {
        InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
    }
    else if (UIName == "Menual")
    {
        MenualUi->SetVisibility(ESlateVisibility::Hidden);
    }
    else if (UIName == "Dialogue")
    {
       DialogueUi->SetVisibility(ESlateVisibility::Hidden);
    }
    else if (UIName == "Phone")
    {
        PhoneUi->SetVisibility(ESlateVisibility::Hidden);
    }
}

bool AHotel_Walker::IsViewUIName(FName UIName)
{
    if (UIName == "Phone")
    {
        if (PhoneUi && PhoneUi->GetVisibility() == ESlateVisibility::Visible)
        {
            return true;
        }
    }
    else if (UIName == "InteractBox")
    {
        if (Interactive_Box && Interactive_Box->GetVisibility() == ESlateVisibility::Visible)
        {
            return true;
        }
    }
    else if (UIName == "InteractMessage")
    {
        if (InteractiveWidget && InteractiveWidget->GetVisibility() == ESlateVisibility::Visible)
        {
            return true;
        }
    }
    else if (UIName == "Menual")
    {
        if (MenualUi && MenualUi->GetVisibility() == ESlateVisibility::Visible)
        {
            return true;
        }
    }
    else if (UIName == "Dialogue")
    {
        if (DialogueUi && DialogueUi->GetVisibility() == ESlateVisibility::Visible)
        {
            return true;
        }
    }

    return false;
}

bool AHotel_Walker::IsInteractThisObject(IInteractable_Object* testObject)
{
    if (interactObject && interactObject == testObject)
    {
        return true;
    }
    return false;
}


void AHotel_Walker::SetCatching(AHotel_Guest* CatchingGuest)
{
    if (isCatchNeck) return;

    isCatchNeck = true;

    if (!bUseControllerRotationYaw)
    {
        GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 자동 회전 X
        bUseControllerRotationYaw = false;
    }
    //PlayerController->ClientStartCameraShake(UBP_CatchShake::StaticClass());

    FVector MyLocation = GetActorLocation();
    FVector TargetLocation = CatchingGuest->GetActorLocation();

    FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLocation, TargetLocation);
    SetActorRotation(FRotator(0, LookAtRot.Yaw, 0)); // 피치와 롤은 무시하고 Yaw만 사용 (수평 회전만)

    PlayerController->SetControlRotation(LookAtRot); 
    
    UCapsuleComponent *temp = GetComponentByClass<UCapsuleComponent>();

    temp->SetCollisionProfileName("NoCollision");

    FTimerHandle EndGameTimer;
    GetWorld()->GetTimerManager().SetTimer(EndGameTimer, [this]()
        {
            Hotel_Manager->MinusHRScore(100, TEXT("사망으로 인한 근무지 이탈"));
            Hotel_Manager->SetGameEnd(EGameEndReason::GER_Dead);
        }
    , 3.0f, false);

}

void AHotel_Walker::EnterHotel()
{
    isEnteringHotel = true;
    Hotel_Manager->SettingEvent(); 
    if (MenualUi)
    {
        MenualUi->SetGameStart();
        MenualUi->InitMenualText();
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

    isCatchNeck = true;
    if (!bUseControllerRotationYaw)
    {
        GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 자동 회전 X
        bUseControllerRotationYaw = false;
    }

    if (GetCharacterMovement()) //중력 끄기
    {
        GetCharacterMovement()->GravityScale = 0.0f;
        GetCharacterMovement()->Velocity.Z = 0.0f;
    }

    UCapsuleComponent* temp = GetComponentByClass<UCapsuleComponent>();
    temp->SetCollisionProfileName("NoCollision");
    FVector relativeOffset(0, -5, -80);
    FVector worldPos = RopeTrans.TransformPosition(relativeOffset);
    FRotator EditedRot = RopeTrans.Rotator();
    EditedRot.Pitch -= 20;
    SetActorRotation(FRotator(0, EditedRot.Yaw, 0)); // 피치와 롤은 무시하고 Yaw만 사용 (수평 회전만)
    PlayerController->SetControlRotation(EditedRot);
    TeleportTo(worldPos, RopeTrans.Rotator());


    FTimerHandle EndGameTimer;
    GetWorld()->GetTimerManager().SetTimer(EndGameTimer, [this]()
        {
            Hotel_Manager->MinusHRScore(100, TEXT("근무중 실종"));
            Hotel_Manager->SetGameEnd(EGameEndReason::GER_Dead);
        }
    , 3.0f, false);

}
