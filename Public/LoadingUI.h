// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "LoadingUI.generated.h"

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API ULoadingUI : public UUserWidget
{
	GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UCanvasPanel* FrontPage;
    UPROPERTY(meta = (BindWidget))
    UImage* BackPage;

    void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
};
