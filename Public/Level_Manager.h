// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DirectionalLight.h"
#include "MainMenuUI.h"
#include "LoadingUI.h"
#include "Hotel_Walker.h"
#include "GameEndReportUI.h"
#include "Components/EditableTextBox.h"
#include "Level_Manager.generated.h"

UCLASS()
class BOOTS_ANKLE_HOTEL_API ALevel_Manager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevel_Manager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    APlayerController* PlayerController;

    UHotel_Manager* Hotel_Manager;

public:
    void LoadMainLevel();
    void LoadEndingLevel();

    void ViewLevelUI(int uIType);

    void HideLevelUI(int uiType);

    UFUNCTION()
    void LoadMainComplete();
    UFUNCTION()
    void LoadEndingComplete();

    void RingingBell();
    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    ADirectionalLight* DirectionalLight;


    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UMainMenuUI> MainMenuUiWidgetClass;
    UMainMenuUI* MainMenuUI;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<ULoadingUI> LoadingUIWidgetClass;
    ULoadingUI* LoadingUI;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UGameEndReportUI> EndingUIWidgetClass;
    UGameEndReportUI* EndingUI;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound)
    UAudioComponent* AudioCompBGM;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound)
    UAudioComponent* AudioCompRinging;

    UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Sound)
    USoundBase* SoundEffectMap;
    UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = Sound)
    USoundBase* RingSoundEffect;
};
