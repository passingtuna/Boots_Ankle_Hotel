#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hotel_WalkerLocomotionComponent.generated.h"

struct FInputActionValue;
class UAudioComponent;

DECLARE_DELEGATE_OneParam(FHotelWalkerPlaySoundDelegate, FName);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_WalkerLocomotionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotel_WalkerLocomotionComponent();

	void Initialize(class AHotel_Walker* Walker);

	void ProcessMoveInput(const FInputActionValue& Value);
	void ProcessMoveEnd(const FInputActionValue& Value);
	void ProcessRunStarted(const FInputActionValue& Value);
	void ProcessRunEnded(const FInputActionValue& Value);

	void TryScheduleFootstepSoundEnd(float DelaySeconds = 0.2f);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UFUNCTION()
	void StopFootstepSoundInternal();

	TWeakObjectPtr<class ACharacter> WeakOwnerCharacter;
	TWeakObjectPtr<UAudioComponent> WeakAudioComp;
	FHotelWalkerPlaySoundDelegate PlaySoundDelegate;

	bool bIsRunning = false;
	FTimerHandle FootStepTimer;
};
