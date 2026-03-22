#include "Hotel_WalkerInteractionComponent.h"
#include "Hotel_Walker.h"
#include "Hotel_Guest.h"
#include "Interactable_Object.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

UHotel_WalkerInteractionComponent::UHotel_WalkerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotel_WalkerInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnShowInteractionPrompt.Unbind();
	OnHideInteractionPrompt.Unbind();
	CurrentInteractTarget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void UHotel_WalkerInteractionComponent::Initialize(AHotel_Walker* OwnerWalker)
{
	if (!OwnerWalker)
	{
		return;
	}

	WeakOwnerWalker = OwnerWalker;
	OnShowInteractionPrompt.Unbind();
	OnHideInteractionPrompt.Unbind();
	OnShowInteractionPrompt.BindUObject(OwnerWalker, &AHotel_Walker::ViewUIInteractiveMessageBox);
	OnHideInteractionPrompt.BindUObject(OwnerWalker, &AHotel_Walker::HideInteractionPromptWidget);
}

void UHotel_WalkerInteractionComponent::ClearInteractionFocusForBlockedState()
{
	if (CurrentInteractTarget)
	{
		CurrentInteractTarget->SetHighLightInteractive(false);
	}
	OnHideInteractionPrompt.ExecuteIfBound();
}

void UHotel_WalkerInteractionComponent::TurnOffHighlightOnly()
{
	if (CurrentInteractTarget)
	{
		CurrentInteractTarget->SetHighLightInteractive(false);
	}
}

void UHotel_WalkerInteractionComponent::TickInteraction(float DeltaTime, UCameraComponent* Camera)
{
	AHotel_Walker* Owner = WeakOwnerWalker.Get();
	if (!Owner || !Camera || !GetWorld())
	{
		return;
	}

	InteractionTraceElapsed += DeltaTime;
	if (InteractionTraceElapsed < 0.05f)
	{
		return;
	}
	InteractionTraceElapsed = 0.0f;

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = Start + Camera->GetForwardVector() * 200.f;

	if (CurrentInteractTarget)
	{
		CurrentInteractTarget->SetHighLightInteractive(false);
		OnHideInteractionPrompt.ExecuteIfBound();
		CurrentInteractTarget = nullptr;
	}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit)
	{
		if (AActor* HitActor = Hit.GetActor())
		{
			if (HitActor->GetClass()->ImplementsInterface(UInteractable_Object::StaticClass()))
			{
				CurrentInteractTarget = Cast<IInteractable_Object>(HitActor);
				if (CurrentInteractTarget)
				{
					CurrentInteractTarget->SetHighLightInteractive(true);
					OnShowInteractionPrompt.ExecuteIfBound();
					if (AHotel_Guest* Guest = Cast<AHotel_Guest>(HitActor))
					{
						Guest->CheckWalkerLookingFace(Start, End);
					}
				}
			}
		}
	}
}
