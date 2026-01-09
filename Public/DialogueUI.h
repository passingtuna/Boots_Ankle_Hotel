// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Hotel_Types.h"
#include "Components/Button.h"
#include "DialogueUI.generated.h"

/**
 * 
 */
class AHotel_Walker;
class UHotel_Manager;
class UDialogueChoiceButton;
class AHotel_Guest;
class AAI_Hotel_Guest_Default;
class AHotel_Phone;
class AHotel_Phone;
class UTextBlock;
class UVerticalBox;
class UButton;
class UDialogueDataAsset;

UCLASS()
class BOOTS_ANKLE_HOTEL_API UDialogueUI : public UUserWidget
{
	GENERATED_BODY()

    AHotel_Guest* NowGuest;
    AAI_Hotel_Guest_Default* NowGuestController;

    UDialogueDataAsset* NowDialogueData;
    int NowDialogueIndex;
    int NowFunctionParameter;
    int NowGuestDialogueIndex;

    FString DialogueName;
    AHotel_Walker* Hotel_Walker;
    UHotel_Manager* Hotel_Manager;
    AHotel_Phone* OverlayPhone;
    FTimerHandle TimerHandle;
    bool isPrevDisConnect;
    TArray<UDialogueChoiceButton*>  arrChoiceButton;
    TMap<FName, TFunction<void()>> mapFunction;
    FTimerHandle DialogueTimer;
    TFunction<void()> TimerFuntion;

    UDialogueChoiceButton* WaitButton;
    UDialogueChoiceButton* EndButton;

    bool isPreInitDialogueIndex;
    bool isAlreadyEnd;
    
public:
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UDialogueChoiceButton> UDialogueChoiceButtonClass;

    void SetDialogueGuest(AHotel_Guest* GuestInfo, AHotel_Phone* Phone);
    void ViewDialogue();
    void ViewSelectionDialogue();
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* ChooseButtonContainer;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TextLineBox;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TextNameBox;
    UPROPERTY(meta = (BindWidget))
    UButton* OverlayButton;
    void SetHotelWalker(AHotel_Walker* Walker) { Hotel_Walker = Walker; };
    void ActivateButton(const FDialogueChoice& ChoiceData);
    void DeactivateAllButton();
    void EndDialogue();
    FText GetFormatNamedText(const FString& Text);
    void SetNowIndex(int Index);
    void SetFunctionParameter(int parameter) { NowFunctionParameter = parameter; };
    void RejectFullRooms();
    void CheckIn();
    void UpdateGuestName();
    void SetNextDialogueIndex(int index);

    UFUNCTION()
    void ExecuteTimerFunction();
    UFUNCTION()
    void PauseDialogue();
    void GuestWaitOvertime();
    void OpenDoor();
    void Reporting();
    void SecurityReport();
    void DisconnectCalling();
    void SetDialogueIndex();
    void MinusHRResource();
    void FireWalker();
    void DecereasePatience();
    void CheckRoomCondition();
    void ApologizeAccept();
};


UCLASS()
class UDialogueChoiceButton : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    UButton* Button;
    UPROPERTY(meta = (BindWidget))
    UTextBlock* TextBlock;

    int NextDialogueId;
    int FunctionParameter;
    TFunction<void()> StoredAction;
    UDialogueUI* DialogueWidget;
    
    virtual void NativeConstruct() override
    {
        Super::NativeConstruct();
        if (Button)
        {
            Button->OnClicked.AddDynamic(this, &UDialogueChoiceButton::OnClickedInternal);
        }
    }

    UFUNCTION()
    void OnClickedInternal()
    {
        if (DialogueWidget)
        {
            DialogueWidget->SetFunctionParameter(FunctionParameter);
        }
        if (StoredAction)
        {
            StoredAction();
        }
        if (DialogueWidget)
        {
            DialogueWidget->SetNowIndex(NextDialogueId);
        }
    }
};
