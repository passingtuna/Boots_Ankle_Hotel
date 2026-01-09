// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Hotel_Types.h"
#include "OptionMenuUI.generated.h"

/**
 * 
 */
class UImage;
class UHotel_Manager;
class UCanvasPanel;
UCLASS()
class BOOTS_ANKLE_HOTEL_API UOptionMenuUI : public UUserWidget
{
	GENERATED_BODY()
    UHotel_Manager* Hotel_Manager;
    TArray<FManualInfo> arrMenualInfo;
    int LoadedMenualLevel;
public:
    UFUNCTION(Blueprintcallable)
    void SaveButtonAction();
    UFUNCTION(Blueprintcallable)
    void ExitButtonAction();
    UFUNCTION(Blueprintcallable)
    void CloseButtonAction();

    virtual void NativeConstruct() override;

    UFUNCTION(Blueprintcallable)
    void EnviromentLevelButtonAction(int Level);
    UFUNCTION(Blueprintcallable)
    void MenualLevelButtonAction(int Level);

    void HighLightButtonImage(bool isMenaul,int level);
    int MenualLevel;
    int EnviromentLevel;

    void InitOption();
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* ConfirmPanel;

    UPROPERTY(meta = (BindWidget))
    UImage* MenualLevelImage0;
    UPROPERTY(meta = (BindWidget))
    UImage* MenualLevelImage1;
    UPROPERTY(meta = (BindWidget))
    UImage* MenualLevelImage2;

    UPROPERTY(meta = (BindWidget))
    UImage* EnviromentLevelImage0;
    UPROPERTY(meta = (BindWidget))
    UImage* EnviromentLevelImage1;
    UPROPERTY(meta = (BindWidget))
    UImage* EnviromentLevelImage2;

};
