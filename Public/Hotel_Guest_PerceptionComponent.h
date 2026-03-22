#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hotel_Guest_PerceptionComponent.generated.h"

class AHotel_Guest;
class AHotel_Walker;
class UHotel_Manager;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_Guest_PerceptionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotel_Guest_PerceptionComponent();

	void Initialize(AHotel_Guest* InOwner);
	void TickPerception(float DeltaTime);

	void ClearWalkerLookingAtGuestFlag();

	bool WasWalkerLookingAtGuest() const { return bWasLookingAtGuest; }

private:
	void UpdateLookTargetWhileViewing(AHotel_Guest* Guest, AHotel_Walker* Walker);
	void UpdateNeckShakeCatch(AHotel_Guest* Guest, AHotel_Walker* Walker, float DeltaTime);
	void UpdateWeirdStareUnderLight(AHotel_Guest* Guest, AHotel_Walker* Walker, UHotel_Manager* Manager, float DeltaTime);

protected:
	TWeakObjectPtr<AHotel_Guest> WeakOwner;

	float NeckShakeCheckElapsed = 0.f;
	float WeirdStareCheckElapsed = 0.f;
	bool bWasLookingAtGuest = false;
};
