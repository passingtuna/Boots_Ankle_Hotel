// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interactive_Box.generated.h"

class AHotel_Walker;
class IInteractable_Object;
class UButton;
class AHotel_Walker;
class UVerticalBox;

/**
 * 
 */
UCLASS()
class BOOTS_ANKLE_HOTEL_API UInteractive_Box : public UUserWidget
{
	GENERATED_BODY()

private:
    IInteractable_Object* tempInteractObject;
    UFUNCTION()
    void OnActionButtonClicked();
    UFUNCTION()
    void PressOverlayButtons();
protected:
    void NativeConstruct() override;

public:
    void ShowActionButtons(IInteractable_Object* interactObject);

    UPROPERTY(meta = (BindWidget))
    UVerticalBox* ActionButtonContainer;

    UPROPERTY(meta = (BindWidget))
    UButton* OverlayButton;

    AHotel_Walker* hotel_Walker;
    TMap<UButton *, TFunction<void()>> ButtonMap;

    void SetHotelWalker(AHotel_Walker* Walker) {
        hotel_Walker = Walker;
    };
};
