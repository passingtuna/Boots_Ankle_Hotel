// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Place.h"
#include "Hotel_Walker.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"

// Sets default values
AHotel_Place::AHotel_Place()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    PlaceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = PlaceVolume;

    PlaceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PlaceVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    PlaceVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PlaceVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    PlaceVolume->SetGenerateOverlapEvents(true);
}

// Called when the game starts or when spawned
void AHotel_Place::BeginPlay()
{
	Super::BeginPlay();
    PlaceVolume->OnComponentBeginOverlap.AddDynamic(this, &AHotel_Place::PersonInPlace);
    PlaceVolume->OnComponentEndOverlap.AddDynamic(this, &AHotel_Place::PersonOutPlace);
    ParentPlace = Cast<AHotel_Place>(GetAttachParentActor());
    if (!PlaceName.IsNone())
    {
        GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->AddRegistedPlace(PlaceName, this);
    }
}

// Called every frame
void AHotel_Place::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AHotel_Place::PersonInPlace( UPrimitiveComponent* OverlappedComp, AActor* OtherActor,  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor && OtherActor != this)
    {
        SetOverlappingPerson(OtherActor);
    }
}

void AHotel_Place::AddRegistPlace()
{
    GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->AddRegistedPlace(PlaceName, this);
}

void AHotel_Place::PersonOutPlace(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValid(ParentPlace))
    {
        ParentPlace->SetOverlappingPerson(OtherActor); //
    }
    if (OtherActor->IsA<AHotel_Walker>())
    {
        FString tempString = "Walker_Out_" + PlaceName.ToString();
        GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->OnEventTriggerAction(FName(tempString));
    }
    else if (OtherActor->IsA<AHotel_Guest>())
    {
        AHotel_Guest* tempGuest = Cast<AHotel_Guest>(OtherActor);
        if (tempGuest->IsCheckTrigger)
        {
            FString tempString = tempGuest->GuestName + "_Out_" + PlaceName.ToString();
            GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->OnEventTriggerAction(FName(tempString));
        }
    }
}

void AHotel_Place::UpdateWalkerLocation()
{
    GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->UpdateWalkerLocation(PlaceName);
}

void AHotel_Place::SetOverlappingPerson(AActor* OtherActor)
{
    if(OtherActor->IsA<AHotel_Guest>())
    {
        AHotel_Guest* tempGuest = Cast<AHotel_Guest>(OtherActor);
        if (tempGuest->IsCheckTrigger)
        {
            FString tempString = tempGuest->GuestName + "_In_" + PlaceName.ToString();
            GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->OnEventTriggerAction(FName(tempString));
        }
    }
    else if (OtherActor->IsA<AHotel_Walker>())
    {
        UpdateWalkerLocation();
    }
}
void AHotel_Place::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (IsValid(PlaceVolume))
    {
        PlaceVolume->OnComponentEndOverlap.RemoveDynamic(this, &AHotel_Place::PersonOutPlace);
    }
}
