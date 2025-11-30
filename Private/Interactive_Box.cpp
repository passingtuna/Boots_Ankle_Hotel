// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactive_Box.h"
#include "Components/TextBlock.h"
#include "Interactable_Object.h"
#include "Hotel_Walker.h"
#include "Components/Button.h"


void UInteractive_Box::NativeConstruct()
{
    Super::NativeConstruct();
    OverlayButton->OnClicked.AddDynamic(this, &UInteractive_Box::PressOverlayButtons);
}
void UInteractive_Box::ShowActionButtons(IInteractable_Object* interactObject)
{
    if (!ActionButtonContainer) return;
    ButtonMap.Empty();
    ActionButtonContainer->ClearChildren();
    tempInteractObject = interactObject;
    for (const auto& Elem : tempInteractObject->GetAvailableAction())
    {
        FName ActionKey = Elem.Key;
        TFunction<void()> ActionFunc = Elem.Value;
        UButton* Button = NewObject<UButton>(this);
        UTextBlock* Label = NewObject<UTextBlock>(Button);
        Label->SetText(FText::FromName(ActionKey)); 

        FSlateFontInfo FontInfo = Label->GetFont();
        FontInfo.Size = 16;
        Label->SetFont(FontInfo);

        Button->AddChild(Label);
        ButtonMap.Add(Button, MoveTemp(ActionFunc));

        Button->IsFocusable = true;
        TMap<FName, TFunction<void()>> availableAction;
        Button->OnClicked.AddDynamic(this, &UInteractive_Box::OnActionButtonClicked);
        ActionButtonContainer->AddChild(Button);
    }

    SetVisibility(ESlateVisibility::Visible);
}
void UInteractive_Box::PressOverlayButtons()
{
    if (hotel_Walker)
    {
        hotel_Walker->GetMouseControl();
    }
    SetVisibility(ESlateVisibility::Hidden);
}

void UInteractive_Box::OnActionButtonClicked()
{
    if (!tempInteractObject->GetIsInteractive()) return;
    for (const auto& Elem : ButtonMap)
    {
        if (IsValid(Elem.Key))
        {
            if (Elem.Key->IsHovered())
            {
                PressOverlayButtons();
                Elem.Value();
            }
        }
    }
}