// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Hotel_Types.h"
#include "GameEndReportUI.generated.h"

/**
 * 
 */

class UHotel_Manager;
class UTextBlock;
class UButton;

UCLASS()
class BOOTS_ANKLE_HOTEL_API UGameEndReportUI : public UUserWidget
{
	GENERATED_BODY()

public:
    UHotel_Manager* Hotel_Manager;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ScoreBox;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* ReasonBox;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TitleBox;
    UPROPERTY(meta = (BindWidget))
    UButton* ContinueButton;

    void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    void SetHRReasonText();
    TArray <FHRRecord> arrHRRecord;
    FString strRecordText;

    UFUNCTION(BlueprintCallable)
    void MainMenuButtonAction();
    UFUNCTION(BlueprintCallable)
    void ContinueButtonAction();
};
