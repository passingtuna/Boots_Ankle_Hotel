#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hotel_WalkerEntranceIntroComponent.generated.h"

class AHotel_Walker;
class UHotel_WalkerLocomotionComponent;
class UHotel_WalkerUIComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_WalkerEntranceIntroComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotel_WalkerEntranceIntroComponent();

	void StartIntro();
	bool IsIntroActive() const { return bIntroActive; }

	bool TickIntro(float DeltaTime, AHotel_Walker* Walker, UHotel_WalkerLocomotionComponent* Loco, UHotel_WalkerUIComponent* UI);

	UPROPERTY(EditDefaultsOnly, Category = "Walker|Intro")
	float IntroPitchLowerBound = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Walker|Intro")
	float IntroPitchUpperClamp = 30.f;

	UPROPERTY(EditDefaultsOnly, Category = "Walker|Intro")
	float IntroPitchAdjustSpeed = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Walker|Intro")
	float IntroForwardUntilWorldY = 1400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Walker|Intro")
	float IntroAutoForwardInputScalar = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Walker|Intro")
	FName IntroWalkSoundName = FName(TEXT("Walk"));

protected:
	bool bIntroActive = false;
};
