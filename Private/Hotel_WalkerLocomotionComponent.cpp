#include "Hotel_WalkerLocomotionComponent.h"
#include "Hotel_Walker.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "Components/AudioComponent.h"
#include "InputActionValue.h"

UHotel_WalkerLocomotionComponent::UHotel_WalkerLocomotionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotel_WalkerLocomotionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	PlaySoundDelegate.Unbind();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FootStepTimer);
	}
	Super::EndPlay(EndPlayReason);
}

void UHotel_WalkerLocomotionComponent::Initialize(AHotel_Walker* Walker)
{
	if (!Walker)
	{
		return;
	}

	PlaySoundDelegate.Unbind();
	WeakOwnerCharacter = Walker;
	WeakAudioComp = Walker->GetWalkerAudioComponent();

	PlaySoundDelegate.BindUObject(Walker, &AHotel_Walker::PlaySound);
}

void UHotel_WalkerLocomotionComponent::ProcessMoveInput(const FInputActionValue& Value)
{
	ACharacter* OwnerChar = WeakOwnerCharacter.Get();
	if (!OwnerChar)
	{
		return;
	}

	AController* Controller = OwnerChar->GetController();
	if (!Controller)
	{
		return;
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (!bIsRunning)
	{
		PlaySoundDelegate.ExecuteIfBound(FName(TEXT("Walk")));
		MovementVector *= 0.6f;
	}
	else
	{
		PlaySoundDelegate.ExecuteIfBound(FName(TEXT("Run")));
	}

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	OwnerChar->AddMovementInput(ForwardDirection, MovementVector.Y);
	OwnerChar->AddMovementInput(RightDirection, MovementVector.X);
}

void UHotel_WalkerLocomotionComponent::ProcessMoveEnd(const FInputActionValue& Value)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(FootStepTimer, this, &UHotel_WalkerLocomotionComponent::StopFootstepSoundInternal, 0.2f, false);
	}
}

void UHotel_WalkerLocomotionComponent::ProcessRunStarted(const FInputActionValue& Value)
{
	bIsRunning = true;
}

void UHotel_WalkerLocomotionComponent::ProcessRunEnded(const FInputActionValue& Value)
{
	bIsRunning = false;
}

void UHotel_WalkerLocomotionComponent::TryScheduleFootstepSoundEnd(float DelaySeconds)
{
	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(FootStepTimer))
		{
			World->GetTimerManager().SetTimer(FootStepTimer, this, &UHotel_WalkerLocomotionComponent::StopFootstepSoundInternal, DelaySeconds, false);
		}
	}
}

void UHotel_WalkerLocomotionComponent::StopFootstepSoundInternal()
{
	if (UAudioComponent* AC = WeakAudioComp.Get())
	{
		AC->Stop();
		AC->SetSound(nullptr);
	}
}
