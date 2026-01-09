// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuUI.generated.h"

/**
 * 
 */
class UOptionMenuUI;
class AHotel_Walker;
class UCanvasPanel;
class UTextBlock;

UCLASS()
class BOOTS_ANKLE_HOTEL_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()
private:
    AHotel_Walker* Hotel_Walker;
    float DayOpacity;
    int WalkingDay;

    int testint = 0;
public:
    void NativeConstruct() override;
    void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = UI)
    TSubclassOf<UOptionMenuUI> OptionUIWidgetClass;
    UOptionMenuUI* OptionUI;

    void SetHotelWalker(AHotel_Walker* hotel_Walker) { Hotel_Walker = hotel_Walker; };
    UFUNCTION(Blueprintcallable)
    void EnterButtonAction();
    UFUNCTION(Blueprintcallable)
    void OptionButtonAction();
    UFUNCTION(Blueprintcallable)
    void ExitButtonAction();
    UFUNCTION(Blueprintcallable)
    void ViewMenualButtonAction();


    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* MainMenuPanel;
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* WalkingDayPanel;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* WalkingDayTextBlock;

};
