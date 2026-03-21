#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hotel_WalkerInteractionComponent.generated.h"

class UCameraComponent;
class IInteractable_Object;

DECLARE_DELEGATE(FHotelWalkerInteractionPromptDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_WalkerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotel_WalkerInteractionComponent();

	void Initialize(class AHotel_Walker* OwnerWalker);

	void ClearInteractionFocusForBlockedState();
	void TickInteraction(float DeltaTime, UCameraComponent* Camera);

	IInteractable_Object* GetCurrentInteractable() const { return CurrentInteractTarget; }
	bool IsTargeting(IInteractable_Object* TestObject) const { return CurrentInteractTarget != nullptr && CurrentInteractTarget == TestObject; }

	void TurnOffHighlightOnly();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	TWeakObjectPtr<class AHotel_Walker> WeakOwnerWalker;
	IInteractable_Object* CurrentInteractTarget = nullptr;

	float InteractionTraceElapsed = 0.0f;

	FHotelWalkerInteractionPromptDelegate OnShowInteractionPrompt;
	FHotelWalkerInteractionPromptDelegate OnHideInteractionPrompt;
};
