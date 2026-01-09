// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Object.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Hotel_Manager.h"
#include "Hotel_Walker.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

// Sets default values
AHotel_Object::AHotel_Object()
{
	PrimaryActorTick.bCanEverTick = true;
    QuickActionName = "없음";
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));

    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));

    AudioComp->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AHotel_Object::BeginPlay()
{
    Hotel_Walker = Cast<AHotel_Walker>(GetWorld()->GetFirstPlayerController()->GetPawn());

    Mesh = Cast<UStaticMeshComponent>(GetComponentByClass(UStaticMeshComponent::StaticClass()));
    Mesh->SetRenderCustomDepth(true);
    SetDefaultInteractiveMessage();

    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    Super::BeginPlay();
}
void AHotel_Object::SetDefaultInteractiveMessage()
{
    FString TempString;
    if (availableAction.Num() > 0)
    {
        TempString += TEXT("[우클릭] 상호작용 메뉴 : ") + ObjectName;
    }
    if (QuickActionName != "없음")
    {
        TempString += TEXT("\n[E] : 퀵 액션 : ") + QuickActionName.ToString();
    }
    InteractMassage = TempString;
}

// Called every frame
void AHotel_Object::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHotel_Object::ExecuteActionByName(FName ActionName)
{
    if (!IsInteractable) return;//상호작용 불가능 상태라면
    if (TFunction<void()>* Func = availableAction.Find(ActionName))
    {
        (*Func)();
    }
}

void AHotel_Object::SetHighLightInteractive(bool OnOff)
{
    if (Mesh)
    {
        if (OnOff)
        {
            Mesh->SetCustomDepthStencilValue(1);
        }
        else
        {
            Mesh->SetCustomDepthStencilValue(0);
        }
    }
      //#include "AudioDevice.h"
    //UGameplayStatics::PlaySoundAtLocation(this, MySoundCue, GetActorLocation());
}


void AHotel_Object::ExecuteQuickAction()
{
    ExecuteActionByName(QuickActionName);
}


FString AHotel_Object::GetInteractMessage()
{
    return InteractMassage;
}
 FName  AHotel_Object::GetQuickActionName()
{
     return QuickActionName;
}

void AHotel_Object::PlaySound(FName SoundKey)
{
    USoundBase* sb_Sound;

    if (SoundEffectMap.Contains(SoundKey))
    {
        sb_Sound = *SoundEffectMap.Find(SoundKey);
       // if (AudioComp->GetSound() != sb_Sound)
        {
            AudioComp->SetSound(sb_Sound);
            AudioComp->Play();
        }
    }
}

void AHotel_Object::StopSound()
{
    AudioComp->SetSound(NULL);
    AudioComp->Stop();
}
