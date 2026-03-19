#include "Hotel_Place.h"
#include "Hotel_Walker.h"
#include "Hotel_Guest_Room.h"
#include "Hotel_Manager.h"
#include "Hotel_Guest.h"
#include "Hotel_Switch.h"
#include "Hotel_CCTV_Camera.h"
#include "Components/BoxComponent.h"

AHotel_Place::AHotel_Place()
{
	PrimaryActorTick.bCanEverTick = true;

    PlaceVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = PlaceVolume;

    PlaceVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PlaceVolume->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    PlaceVolume->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PlaceVolume->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    PlaceVolume->SetGenerateOverlapEvents(true);
}

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
        ParentPlace->SetOverlappingPerson(OtherActor);
    }
    if (OtherActor->IsA<AHotel_Walker>())
    {
        GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->OnEventTriggerAction(
            FHotelTrigger::Make(EHotelTriggerType::PlaceStateChange,
                {
                    { EHotelTriggerKey::Instigator, TEXT("Walker") },
                    { EHotelTriggerKey::Place, PlaceName.ToString() },
                    { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::Out) },
                }));
    }
    else if (OtherActor->IsA<AHotel_Guest>())
    {
        AHotel_Guest* tempGuest = Cast<AHotel_Guest>(OtherActor);
        if (tempGuest->IsCheckTrigger)
        {
            GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->OnEventTriggerAction(
                FHotelTrigger::Make(EHotelTriggerType::PlaceStateChange,
                    {
                        { EHotelTriggerKey::Instigator, tempGuest->GuestName },
                        { EHotelTriggerKey::Place, PlaceName.ToString() },
                        { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::Out) },
                    }));
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
            GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->OnEventTriggerAction(
                FHotelTrigger::Make(EHotelTriggerType::PlaceStateChange,
                    {
                        { EHotelTriggerKey::Instigator, tempGuest->GuestName },
                        { EHotelTriggerKey::Place, PlaceName.ToString() },
                        { EHotelTriggerKey::ObjectState, HotelTriggerStateToString(EHotelObjectState::In) },
                    }));
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
