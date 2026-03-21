#include "Hotel_WalkerPeepingComponent.h"
#include "Hotel_Walker.h"
#include "Hotel_Door.h"
#include "GameFramework/PlayerController.h"

UHotel_WalkerPeepingComponent::UHotel_WalkerPeepingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotel_WalkerPeepingComponent::Initialize(AHotel_Walker* Owner)
{
	WeakOwner = Owner;
	bIsPeeping = false;
	ActivePeepingDoor = nullptr;
}

void UHotel_WalkerPeepingComponent::SetPeepingState(bool bPeeping, AHotel_Door* PeepingDoor)
{
	bIsPeeping = bPeeping;
	ActivePeepingDoor = bPeeping ? PeepingDoor : nullptr;
}

void UHotel_WalkerPeepingComponent::StopPeeping()
{
	if (!bIsPeeping)
	{
		return;
	}

	if (AHotel_Walker* Owner = WeakOwner.Get())
	{
		if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
		{
			PC->SetViewTargetWithBlend(Owner);
		}
	}

	bIsPeeping = false;
	if (ActivePeepingDoor)
	{
		ActivePeepingDoor->PeepingEnd();
		ActivePeepingDoor = nullptr;
	}
}

void UHotel_WalkerPeepingComponent::ApplyPeepingLook(const FVector2D& LookAxis) const
{
	if (ActivePeepingDoor)
	{
		ActivePeepingDoor->MovePeepingEye(LookAxis);
	}
}
