#include "Hotel_Guest_PerceptionComponent.h"
#include "Hotel_Guest.h"
#include "Hotel_Walker.h"
#include "Hotel_Manager.h"
#include "Hotel_Types.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"

UHotel_Guest_PerceptionComponent::UHotel_Guest_PerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotel_Guest_PerceptionComponent::Initialize(AHotel_Guest* InOwner)
{
	WeakOwner = InOwner;
	NeckShakeCheckElapsed = 0.f;
	WeirdStareCheckElapsed = 0.f;
	bWasLookingAtGuest = false;
}

void UHotel_Guest_PerceptionComponent::ClearWalkerLookingAtGuestFlag()
{
	bWasLookingAtGuest = false;
}

void UHotel_Guest_PerceptionComponent::UpdateLookTargetWhileViewing(AHotel_Guest* Guest, AHotel_Walker* Walker)
{
	if (Walker && Guest->IsLookingPlayer)
	{
		Guest->SetLookActor(Walker);
	}
}

void UHotel_Guest_PerceptionComponent::UpdateNeckShakeCatch(AHotel_Guest* Guest, AHotel_Walker* Walker, float DeltaTime)
{
	if (!Walker || !Guest->IsReadyToNeckShaking || Guest->IsAlreadyNeckShaking)
	{
		return;
	}

	NeckShakeCheckElapsed += DeltaTime;
	if (NeckShakeCheckElapsed < 0.05f)
	{
		return;
	}
	NeckShakeCheckElapsed = 0.f;

	const float Distance = FVector::Dist(Guest->GetActorLocation(), Walker->GetActorLocation());
	if (Distance >= 200.f)
	{
		return;
	}

	Guest->CatchingPlayer();
	if (Guest->IsWierdHanging && Guest->Hotel_Manager)
	{
		Guest->Hotel_Manager->UpdateDefualtLevelMenual(5);
	}
}

void UHotel_Guest_PerceptionComponent::UpdateWeirdStareUnderLight(
	AHotel_Guest* Guest,
	AHotel_Walker* Walker,
	UHotel_Manager* Manager,
	float DeltaTime)
{
	if (!Walker || !Manager || !Guest->IsWierdStareUnderLight)
	{
		return;
	}

	UCameraComponent* WalkerCamera = Walker->FollowCamera;
	USphereComponent* FaceCol = Guest->FaceCollision;
	UCapsuleComponent* Capsule = Guest->CapsuleComponent;
	if (!WalkerCamera || !FaceCol || !Capsule || !Guest->GetWorld())
	{
		return;
	}

	WeirdStareCheckElapsed += DeltaTime;
	if (WeirdStareCheckElapsed < 0.1f)
	{
		return;
	}
	WeirdStareCheckElapsed = 0.f;

	FHitResult Hits;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Guest);
	Params.AddIgnoredActor(Walker);
	if (Guest->AutoDoor)
	{
		Params.AddIgnoredActor(Guest->AutoDoor);
	}
	Params.AddIgnoredComponent(Capsule);

	FVector Start = Guest->GetActorLocation();
	FVector End = Walker->GetActorLocation();
	Start.Z += 67.f;
	End.Z += 67.f;

	bool bIsLookingAtGuest = false;
	const bool bHit = Guest->GetWorld()->LineTraceSingleByChannel(Hits, Start, End, ECC_Visibility, Params);
	if (!bHit)
	{
		const FVector TempVec1 = Start - End;
		const FVector TempVec2 = WalkerCamera->GetForwardVector();

		FVector TempVec3 = TempVec1;
		TempVec3.Z = 0.f;
		FVector TempVec4 = TempVec2;
		TempVec4.Z = 0.f;

		const float Dot = FVector::DotProduct(TempVec3.GetSafeNormal(), TempVec4.GetSafeNormal());
		if (Dot > 0.7f)
		{
			const float DeltaZ = FaceCol->GetComponentLocation().Z + 10.f;
			const float HorizontalDist = FVector(Guest->GetActorLocation() - Walker->GetActorLocation()).Size2D();
			const float VerticalAngle = FMath::RadiansToDegrees(FMath::Atan2(DeltaZ, HorizontalDist));
			const float CameraPitch = WalkerCamera->GetComponentRotation().Pitch;

			const float UpperLimit = 0.32f * VerticalAngle + 27.4f;
			const float LowerLimit = -1.12f * VerticalAngle - 27.4f;

			bIsLookingAtGuest = (CameraPitch >= LowerLimit && CameraPitch <= UpperLimit);
		}
	}

	if (bIsLookingAtGuest != bWasLookingAtGuest)
	{
		if (bIsLookingAtGuest)
		{
			Manager->OnEventTriggerAction(
				FHotelTrigger::Make(EHotelTriggerType::LookStateChange,
					{
						{ EHotelTriggerKey::Instigator, TEXT("Walker") },
						{ EHotelTriggerKey::Target, Guest->GuestName },
						{ EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::Look) },
					}));
		}
		else
		{
			Manager->OnEventTriggerAction(
				FHotelTrigger::Make(EHotelTriggerType::LookStateChange,
					{
						{ EHotelTriggerKey::Instigator, TEXT("Walker") },
						{ EHotelTriggerKey::Target, Guest->GuestName },
						{ EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::NoLook) },
					}));
		}

		bWasLookingAtGuest = bIsLookingAtGuest;
	}
}

void UHotel_Guest_PerceptionComponent::TickPerception(float DeltaTime)
{
	AHotel_Guest* Guest = WeakOwner.Get();
	if (!Guest)
	{
		return;
	}

	AHotel_Walker* Walker = Guest->Hotel_Walker;
	UpdateLookTargetWhileViewing(Guest, Walker);
	UpdateNeckShakeCatch(Guest, Walker, DeltaTime);

	UHotel_Manager* Manager = Guest->Hotel_Manager;
	UpdateWeirdStareUnderLight(Guest, Walker, Manager, DeltaTime);
}
