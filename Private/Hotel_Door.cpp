// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Door.h"
#include "Hotel_Walker.h"
#include "Hotel_Guest.h"
#include "Hotel_Manager.h"
#include "Components/TextRenderComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/SpringArmComponent.h"

AHotel_Door::AHotel_Door()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AHotel_Door::SetDoorMovementActive(bool bActive)
{
	isMoving = bActive;
	SetActorTickEnabled(bActive);
}

void AHotel_Door::BeginPlay()
{
    Super::BeginPlay();
    ObjectName = DoorName.ToString() + TEXT(" 문");
    availableAction.Add(TEXT("열기/닫기"), [this]() { ToggleOpenByWalker(); });
    availableAction.Add(TEXT("잠금/해제"), [this]() { ToggleLockByWalker(); });
    availableAction.Add(TEXT("훔쳐보기"), [this]() {  Peeping(); });
    availableAction.Add(TEXT("노크"), [this]() {  Knock(); });
    isOpen = false;
    isLock = false;
    isInside = false;
    SetDoorMovementActive(false);

    isActivePeepingFaceInside = false;
    isActivePeepingFaceOutside = false;

    isUncontrolable = false;

    QuickActionName = TEXT("열기/닫기");

    if (DoorName == "Staff") Hotel_Manager->SetStaffDoor(this);

    SetDefaultInteractiveMessage();
}

void AHotel_Door::Tick(float DeltaTime)
{
    if (!isMoving) return;

    if (isOpen)
    {
        if (isInside)
        {
            if (Hinge->GetRelativeRotation().Yaw < 90)
            {
                Hinge->SetRelativeRotation(FRotator(0.0f, Hinge->GetRelativeRotation().Yaw + 6.0f, 0.0f));
            }
            else
            {
                Hinge->SetRelativeRotation(FRotator(0.0f, 90, 0.0f));
                SetDoorMovementActive(false);
            }
        }
        else
        {
            if (Hinge->GetRelativeRotation().Yaw > -90)
            {
                Hinge->SetRelativeRotation(FRotator(0.0f, Hinge->GetRelativeRotation().Yaw - 6.0f, 0.0f));
            }
            else
            {
                Hinge->SetRelativeRotation(FRotator(0.0f, -90, 0.0f));
                SetDoorMovementActive(false);
            }
        }
    } 
    else
    {
        if (Hinge->GetRelativeRotation().Yaw > 0)
        {
            Hinge->SetRelativeRotation(FRotator(0.0f, Hinge->GetRelativeRotation().Yaw - 6.0f, 0.0f));
            if (Hinge->GetRelativeRotation().Yaw < 0)
            {
                Hinge->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
                SetDoorMovementActive(false);
                PlaySound("Clash");
                DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
        else if (Hinge->GetRelativeRotation().Yaw < 0)
        {
            Hinge->SetRelativeRotation(FRotator(0.0f, Hinge->GetRelativeRotation().Yaw + 6.0f, 0.0f));
            if (Hinge->GetRelativeRotation().Yaw > 0)
            {
                Hinge->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
                SetDoorMovementActive(false);
                PlaySound("Clash");
                DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                DoorMesh->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }

    }

}

void AHotel_Door::ToggleOpenByWalker()
{
    if (isUncontrolable)//제어 불가면 문열기/닫기 불가 안됨
    {
        PlaySound("Lock");
        return;
    }

    CheckInteractLocation();
    if (isOpen)
    {
        PlaySound("Close");
        isOpen = false;
        SetDoorMovementActive(true);
    }
    else
    {
        if (!isLock)
        {
            PlaySound("Open");
            isOpen = true;
            SetDoorMovementActive(true);
            DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DoorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            DoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        }
        else
        {
            PlaySound("Lock");
        }
    }
    if (Hotel_Walker)
    {
        Hotel_Walker->StopPeeping();
    }

    Hotel_Manager->OnEventTriggerAction(
        FHotelTrigger::Make(
            EHotelTriggerType::DoorStateChange,
            {
                { EHotelTriggerKey::Place, DoorName.ToString() },
                { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(isOpen ? EHotelObjectState::Open : EHotelObjectState::Close) },
            }));
}
void AHotel_Door::ToggleLockByWalker()
{
    if (isUncontrolable)//제어 불가면 문열기도 안됨
    {
        PlaySound("Lock");
        return;
    }
    if (isOpen)
    {
        return;
    }
    isLock = !isLock;
    isLock ? PlaySound("Lock") : PlaySound("Unlock");

    // Lock / Unlock도 DoorStateChange로 트리거 전송
    Hotel_Manager->OnEventTriggerAction(
        FHotelTrigger::Make(
            EHotelTriggerType::DoorStateChange,
            {
                { EHotelTriggerKey::Place, DoorName.ToString() },
                { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(isLock ? EHotelObjectState::Lock : EHotelObjectState::UnLock) },
            }));
}

void AHotel_Door::ToggleOpen()
{
    if (isUncontrolable)//제어 불가면 문열기/닫기 불가 안됨
    {
        PlaySound("Lock");
        return;
    }

    CheckInteractLocation();
    if (isOpen)
    {
        PlaySound("Close");
        isOpen = false;
        SetDoorMovementActive(true);
    }
    else
    {
        if (!isLock)
        {
            PlaySound("Open");
            isOpen = true;
            SetDoorMovementActive(true);
            DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            DoorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
            DoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
        }
        else
        {
            PlaySound("Lock");
        }
    }
    if (Hotel_Walker)
    {
        Hotel_Walker->StopPeeping();
    }
}

void AHotel_Door ::SetDoorNameText(FName roomName)
{
    DoorName = roomName;
    ObjectName = DoorName.ToString() + TEXT(" 문");
    SetDefaultInteractiveMessage();
    DoorNameText->SetText(FText::FromString(DoorName.ToString()));
}

void AHotel_Door::ToggleLock()
{
    if (isUncontrolable)//제어 불가면 문열기도 안됨
    {
        PlaySound("Lock");
        return;
    }
    if (isOpen)
    {
        return;
    }
    isLock = !isLock;
    isLock ? PlaySound("Lock") : PlaySound("Unlock");
}

void AHotel_Door::SetLockDoor(bool state)
{
    if (isLock != state) ToggleLock();
}

void AHotel_Door::SetOpenDoor(bool state)
{
    if (isOpen != state) ToggleOpen();
}
void AHotel_Door::CheckInteractLocation()
{
    float OutDistance = FVector::Dist(Hotel_Walker->GetActorLocation(), PeepingHoleOutside->GetComponentLocation());
    float InDistance = FVector::Dist(Hotel_Walker->GetActorLocation(), PeepingHoleInside->GetComponentLocation());

    if (OutDistance > InDistance) isInside = false;
    else isInside = true;
}

void AHotel_Door::Peeping()
{
    if (isOpen) return; //열린문은 훔쳐볼수 없음
    CheckInteractLocation();

    if (!isInside)
    {
        PeepingHoleOutside->SetVisibility(true,true);
        GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(this);
        PeepingCameraOutside->Activate();
        PeepingFaceMeshOutside->SetVisibility(isActivePeepingFaceOutside);
        Hotel_Walker->SetPeeping(true, this);
        Hotel_Manager->OnEventTriggerAction(
            FHotelTrigger::Make(
                EHotelTriggerType::PeepingDoor,
                {
                    { EHotelTriggerKey::Place, DoorName.ToString() },
                    { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::Start) },
                }));
    }
    else
    {
        PeepingHoleInside->SetVisibility(true, true);
        GetWorld()->GetFirstPlayerController()->SetViewTargetWithBlend(this);
        PeepingCameraInside->Activate();
        PeepingFaceMeshInside->SetVisibility(isActivePeepingFaceInside);
        Hotel_Walker->SetPeeping(true, this);
        Hotel_Manager->OnEventTriggerAction(
            FHotelTrigger::Make(
                EHotelTriggerType::PeepingDoor,
                {
                    { EHotelTriggerKey::Place, DoorName.ToString() },
                    { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::Start) },
                }));
    }
    PlaySound("Peep");
}

void AHotel_Door::PeepingEnd()
{
    PeepingHoleInside->SetVisibility(false, true);
    PeepingHoleOutside->SetVisibility(false, true);
    PeepingCameraInside->Deactivate();
    PeepingCameraOutside->Deactivate();
    Hotel_Manager->OnEventTriggerAction(
        FHotelTrigger::Make(
            EHotelTriggerType::PeepingDoor,
            {
                { EHotelTriggerKey::Place, DoorName.ToString() },
                { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::End) },
            }));
}

void AHotel_Door::MovePeepingEye(FVector2D LookAxisVector)
{
    FRotator NewRotation = GetActorRotation();
    
    if (isInside)
    {
        NewRotation.Yaw = FMath::Clamp(PeepingArmInside->GetRelativeRotation().Yaw + LookAxisVector.X, -9.0f, 9.0f);
        NewRotation.Roll = FMath::Clamp(PeepingArmInside->GetRelativeRotation().Roll - LookAxisVector.Y, -9.0f, 9.0f);
        PeepingArmInside->SetRelativeRotation(NewRotation);
    }
    else
    {
        NewRotation.Yaw = FMath::Clamp(PeepingArmOutside->GetRelativeRotation().Yaw + LookAxisVector.X, -9.0f, 9.0f);
        NewRotation.Roll = FMath::Clamp(PeepingArmOutside->GetRelativeRotation().Roll - LookAxisVector.Y, -9.0f, 9.0f);
        PeepingArmOutside->SetRelativeRotation(NewRotation);
    }
}

void AHotel_Door::Knock(int Type)
{
    switch (Type)
    {
        case 0:
            PlaySound("Knock2");
            break;
        case 1:
            PlaySound("Knock1");
            break;
        default:
            break;
    }
    Hotel_Manager->KnockingRoomDoor(DoorName);
}


void AHotel_Door::OnOpenTriggerBoxBegin(AActor* OtherActor)
{
    if (IsValid(OtherActor) && OtherActor != this && OtherActor->IsA<AHotel_Guest>())
    {
        AHotel_Guest* TempGuest = Cast<AHotel_Guest>(OtherActor);
        if (!isOpen && TempGuest->GetAutoActionDoor())
        {
            GuestOpen(TempGuest);
        }
    }
}

void AHotel_Door::EndOpenTriggerBoxBegin(AActor* OtherActor)
{
    if (IsValid(OtherActor) && OtherActor != this && OtherActor->IsA<AHotel_Guest>())
    {
        AHotel_Guest * TempGuest = Cast<AHotel_Guest>(OtherActor);
        if (isOpen && TempGuest->GetAutoActionDoor()) GuestClose(TempGuest);
    }

}

void AHotel_Door::GuestOpen(AHotel_Guest* Guest)
{
    float OutDistance = FVector::Dist(Guest->GetActorLocation(), PeepingHoleOutside->GetComponentLocation());
    float InDistance = FVector::Dist(Guest->GetActorLocation(), PeepingHoleInside->GetComponentLocation());
    if (OutDistance > InDistance) isInside = false;
    else isInside = true;

    if (!isLock || (isLock && Hotel_Manager->CheckCorrectGuest(DoorName, Guest)))
    {
        PlaySound("Open");
        isOpen = true;
        SetDoorMovementActive(true);
        DoorMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        DoorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
        DoorMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
    else
    {
        PlaySound("Lock");
    }

    if (Hotel_Walker)
    {
        Hotel_Walker->StopPeeping();
    }
}
void AHotel_Door::GuestClose(AHotel_Guest* Guest)
{
    float OutDistance = FVector::Dist(Guest->GetActorLocation(), PeepingHoleOutside->GetComponentLocation());
    float InDistance = FVector::Dist(Guest->GetActorLocation(), PeepingHoleInside->GetComponentLocation());

    if (OutDistance > InDistance) isInside = false;
    else isInside = true;

    if (isOpen)
    {
        PlaySound("Close");
        isOpen = false;
        SetDoorMovementActive(true);
    }
}