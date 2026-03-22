#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hotel_Door.h"
#include "Hotel_WalkerPeepingComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_WalkerPeepingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotel_WalkerPeepingComponent();

	void Initialize(class AHotel_Walker* Owner);

	void SetPeepingState(bool bPeeping, AHotel_Door* PeepingDoor);
	void StopPeeping();

	bool IsPeeping() const { return bIsPeeping; }
	AHotel_Door* GetActivePeepingDoor() const { return ActivePeepingDoor; }

	void ApplyPeepingLook(const FVector2D& LookAxis) const;

protected:
	TWeakObjectPtr<class AHotel_Walker> WeakOwner;
	bool bIsPeeping = false;
	TObjectPtr<AHotel_Door> ActivePeepingDoor;
};
