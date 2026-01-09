// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuUI.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Hotel_Walker.h"
#include "Components/Button.h"
#include "OptionMenuUI.h"
#include "Components/CanvasPanel.h" 
#include "Components/TextBlock.h"
#include "Hotel_Manager.h"


void UMainMenuUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (OptionUIWidgetClass)
    {
        OptionUI = CreateWidget<UOptionMenuUI>(GetWorld()->GetFirstPlayerController(), OptionUIWidgetClass);
        if (OptionUI)
        {
            OptionUI->AddToViewport(10);
            OptionUI->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    WalkingDayPanel->SetVisibility(ESlateVisibility::Collapsed);
    DayOpacity = 1.0f; 
}

void UMainMenuUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (WalkingDayPanel->GetVisibility() == ESlateVisibility::Visible)
    {
        WalkingDayPanel->SetRenderOpacity(DayOpacity);
        DayOpacity -= 0.015f;
        if (DayOpacity < 0)
        {
            SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UMainMenuUI::EnterButtonAction()
{
    DayOpacity = 1.0f;
    MainMenuPanel->SetVisibility(ESlateVisibility::Collapsed);
    WalkingDayPanel->SetVisibility(ESlateVisibility::Visible);

    WalkingDay = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->GetWalkingDay();

    FString tempString = FString::FromInt(WalkingDay);
    WalkingDayTextBlock->SetText(FText::FromString(tempString));

    Hotel_Walker->EnterHotel();
}
void UMainMenuUI::OptionButtonAction()
{
    if (OptionUI)
    {
        Hotel_Walker->GiveMouseControl(OptionUI);
        OptionUI->SetVisibility(ESlateVisibility::Visible);
        OptionUI->InitOption();
    }
}
void UMainMenuUI::ExitButtonAction()
{
    testint++;
    GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>()->UpdateDefualtLevelMenual(testint);
    /*
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    UKismetSystemLibrary::QuitGame(
        GetWorld(),
        PC,
        EQuitPreference::Quit,
        true
    );*/
}

void UMainMenuUI::ViewMenualButtonAction()
{
    Hotel_Walker->ViewUIMenual();
}