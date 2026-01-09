// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable_Object.h"
#include "Hotel_Object.generated.h"

class AHotel_Walker;
class UHotel_Manager;
class UAudioComponent;
UCLASS()
class BOOTS_ANKLE_HOTEL_API AHotel_Object : public AActor , public IInteractable_Object
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHotel_Object();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
    bool IsInteractable = true;
    FString InteractMassage;
    FString ObjectName;
    TMap<FName, TFunction<void()>> availableAction;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sound, meta = (AllowPrivateAccess = "true"))
    TMap<FName, USoundBase*> SoundEffectMap;

    AHotel_Walker* Hotel_Walker;
    FName QuickActionName;

    UStaticMeshComponent* Mesh;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound)
    UAudioComponent* AudioComp;

    UHotel_Manager* Hotel_Manager;

    ACharacter* InteractedHuman;
	// Called every frame
	virtual void Tick(float DeltaTime) override;
    virtual TMap<FName, TFunction<void()>> GetAvailableAction() override { return availableAction; };
	virtual void ExecuteActionByName(FName ActionName) override;
    virtual void SetHighLightInteractive(bool OnOff) override;
    virtual void ExecuteQuickAction()override;
    void PlaySound(FName SoundKey);
    void StopSound();

    virtual FString GetInteractMessage()override;
    virtual FName  GetQuickActionName()override;

    void SetDefaultInteractiveMessage();
    void SetInteractiveMessage(FString EditedMessage) {InteractMassage = EditedMessage;};

    virtual bool GetIsInteractive()override { return IsInteractable; };
    virtual void SetIsInteractive(bool on)override { IsInteractable = on; };
};
