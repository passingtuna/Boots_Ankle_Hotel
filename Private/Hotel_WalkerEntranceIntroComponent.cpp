#include "Hotel_WalkerEntranceIntroComponent.h"
#include "Hotel_Walker.h"
#include "Hotel_WalkerLocomotionComponent.h"
#include "Hotel_WalkerUIComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"

UHotel_WalkerEntranceIntroComponent::UHotel_WalkerEntranceIntroComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotel_WalkerEntranceIntroComponent::StartIntro()
{
	bIntroActive = true;
}

bool UHotel_WalkerEntranceIntroComponent::TickIntro(float DeltaTime, AHotel_Walker* Walker,
	UHotel_WalkerLocomotionComponent* Loco, UHotel_WalkerUIComponent* UI)
{
	(void)DeltaTime;
	if (!bIntroActive || !Walker)
	{
		return false;
	}

	AController* Ctrl = Walker->GetController();
	if (!Ctrl)
	{
		return true;
	}

	FRotator MyRotator = Ctrl->GetControlRotation();
	if (MyRotator.Pitch > IntroPitchLowerBound)
	{
		Walker->AddControllerPitchInput(IntroPitchAdjustSpeed);
		if (MyRotator.Pitch > IntroPitchUpperClamp)
		{
			MyRotator.Pitch = IntroPitchLowerBound;
			Ctrl->SetControlRotation(MyRotator);
		}
	}
	else if (Walker->GetActorLocation().Y < IntroForwardUntilWorldY)
	{
		const FRotator Rotation = Ctrl->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		Walker->AddMovementInput(ForwardDirection, IntroAutoForwardInputScalar);
		Walker->PlaySound(IntroWalkSoundName);
	}
	else
	{
		if (Loco)
		{
			Loco->TryScheduleFootstepSoundEnd(0.2f);
		}
		if (UI)
		{
			UI->ReleaseMouseToGame();
		}
		bIntroActive = false;
	}

	return true;
}
