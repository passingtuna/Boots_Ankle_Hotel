// Fill out your copyright notice in the Description page of Project Settings.

#include "GameEndReportUI.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Hotel_Manager.h"

void UGameEndReportUI::SetHRReasonText()
{
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    arrHRRecord = Hotel_Manager->GetHRRecord();

    FString tempString;
    for (auto& temp : arrHRRecord) 
    {
        tempString += temp.Reason;
        tempString += temp.Minus ? TEXT("(-") : TEXT("(+");
        tempString += FString::FromInt(temp.Score) + TEXT(")\n");
    }
    ReasonBox->SetText(FText::FromString(tempString));

    int HRScore = Hotel_Manager->GetHRScore();
    if (HRScore <= 0)
    {
        TitleBox->SetText(FText::FromString(TEXT("해고 통지서")));
        ContinueButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        TitleBox->SetText(FText::FromString(TEXT("야간 근무 보고서")));
        ContinueButton->SetVisibility(ESlateVisibility::Visible);
    }
    tempString = "";
    tempString = FString::FromInt(HRScore);
    tempString += "/100";
    ScoreBox->SetText(FText::FromString(tempString));

}
void UGameEndReportUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

}

void UGameEndReportUI::MainMenuButtonAction()
{
    Hotel_Manager->SettingInitGame(false);
    Hotel_Manager->SetMainMenu();
}

void UGameEndReportUI::ContinueButtonAction()
{
    if (Hotel_Manager->GetHRScore())
    {
        Hotel_Manager->SettingInitGame(true);
        Hotel_Manager->SetMainMenu();
    }
}