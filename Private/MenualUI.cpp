// Fill out your copyright notice in the Description page of Project Settings.


#include "MenualUI.h"
#include "Fonts/FontMeasure.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/SlateApplication.h"
#include "Hotel_Walker.h"
#include "Hotel_Menual.h"
#include "Components/VerticalBox.h"
#include "Components/EditableTextBox.h"
#include "Hotel_Manager.h"

void UMenualUI::NativeConstruct()
{
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    arrMenualInfo = Hotel_Manager->GetMenualInfo();
    /*for (auto& tempMenualInfo : *arrMenualInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s"), *tempMenualInfo.MenualText.ToString());
    }*/
    NowPage = 0;

    PreMenualLevel = Hotel_Manager->GetMenualLevel();

    arrTextBox.Empty();
    if (LeftPageBox)
    {
        LeftPageBox->ClearChildren();
        for (int i = 0; i < 40; ++i)
        {
            ULineLimitTextBox* newWidget = CreateWidget<ULineLimitTextBox>(this, ULineLimitTextBoxClass);
            FString Temp = FString::Printf(TEXT("%d Line"), i);
            newWidget->TextBox->SetText(FText::FromString(Temp));
            newWidget->TextBox->SetIsReadOnly(true);
            newWidget->BoxNum = i;
            newWidget->MenaulUI = this;
            arrTextBox.Add(newWidget);
            LeftPageBox->AddChild(newWidget);
        }
    }
    if (RightPageBox)
    {
        RightPageBox->ClearChildren();
        for (int i = 0; i < 40; ++i)
        {
            ULineLimitTextBox* newWidget = CreateWidget<ULineLimitTextBox>(this, ULineLimitTextBoxClass);
            FString Temp = FString::Printf(TEXT("%d Line"), i);
            newWidget->TextBox->SetText(FText::FromString(Temp));
            newWidget->TextBox->SetIsReadOnly(true);
            newWidget->BoxNum = i +39;
            newWidget->MenaulUI = this;
            arrTextBox.Add(newWidget);
            RightPageBox->AddChild(newWidget);
        }
    }
    isGameStarted = false;
    InitMenualText();
}
void UMenualUI::InitMenualText()
{
    arrMenualText.Empty();
    if (Hotel_Manager->GetMenualLevel() == 2)
    {
        for (auto& TextLine : arrTextBox)
        {
            TextLine->TextBox->SetIsReadOnly(false);
        }
        
        FString LoadText = Hotel_Manager->GetFioneerMenaulText();
        LoadText.ParseIntoArray(arrMenualText, TEXT("\n"), false);

        return;
    }

    int nMenualNum = 1;
    for (auto& temp : *arrMenualInfo)
    {
        if (temp.EventID == 0 || Hotel_Manager->GetMenualLevel() == 0)
        {
            FString tempString;
            if (temp.Category == 0)
            {
                tempString = "     ";
                tempString += temp.MenualText.ToString();
            }
            else if (temp.Category == 2) // 암구어
            {
                tempString = FString::Printf(TEXT("%s"),  *Hotel_Manager->GetCodeWord());
            }
            else if (temp.Category == 3) // 예약 손님
            {
                tempString = TEXT("예약 손님 : ") + FString::Printf(TEXT("%s"), *Hotel_Manager->GetReservationGuestName());
            }
            else
            {
                tempString = FString::Printf(TEXT("%d. %s"), nMenualNum, *temp.MenualText.ToString());
                nMenualNum++;
            }
            arrMenualText.Add(tempString);
        }
    }

    if (Hotel_Manager->GetMenualLevel() == 1)
    {
        arrExperiencedEventID = Hotel_Manager->GetExperiencedEventID();
        for (auto& tempEventNum : *arrExperiencedEventID)
        {
            for (auto& tempMenualInfo : *arrMenualInfo)
            {
                if (tempEventNum == tempMenualInfo.EventID)
                {
                    FString tempString;
                    if (tempMenualInfo.Category == 0)
                    {
                        tempString = "     ";
                        tempString += tempMenualInfo.MenualText.ToString();
                    }
                    else
                    {
                        tempString = FString::Printf(TEXT("%d. %s"), nMenualNum, *tempMenualInfo.MenualText.ToString());
                        nMenualNum++;
                    }
                    arrMenualText.Add(tempString);
                }
            }
        }
    }
}
void UMenualUI::InitMenual(AHotel_Walker* hotel_Walker)
{
    Hotel_Walker = hotel_Walker;
    if (!isGameStarted)
    {   
        InitMenualText();
        //PreMenualLevel = NowMenualLevel;
    }
    ShowMenualTextPage();
}
void UMenualUI::NextPageButtonAction()
{
    if (((NowPage) * 80) < arrMenualText.Num() - 1)
    {
        NowPage++;
        ShowMenualTextPage();
    }
}

void UMenualUI::PrevPageButtonAction()
{
    if (NowPage > 0)
    {
        NowPage--;
        ShowMenualTextPage();
    }
}


void UMenualUI::CloseButtonAction()
{
    if (Hotel_Manager->GetMenualLevel())
    {
        Hotel_Manager->SetFioneerMenaulText(MakeFioneerString());
        Hotel_Manager->SaveGameManualExternal();
    }
    if (Hotel_Walker)
    {
        Hotel_Walker->HideUIName("Menual");
        if (isGameStarted)
        {
            Hotel_Walker->GetMouseControl();
        }
    }
}

FString UMenualUI::MakeFioneerString()
{
    FString TempString;

    for (auto& TextLine : arrMenualText)
    {
        TempString += TextLine;
        TempString += "\n";
    }
    return TempString;
}

bool UMenualUI::ShowMenualTextPage()
{
    int startTextNum = NowPage * 80;

    for (auto& TextLine : arrTextBox)
    {
        if (startTextNum < arrMenualText.Num())
        {
            TextLine->TextBox->SetText(FText::FromString(arrMenualText[startTextNum]));
            startTextNum++;
        }
        else
        {
            TextLine->TextBox->SetText(FText::FromString(""));
        }
    }
    return true;
}


void UMenualUI::ChangeTextUI(int BoxNum, const FText& Text, ETextCommit::Type CommitMethod)
{
    int commitBoxNem = (NowPage * 80) + BoxNum;
    if (arrMenualText.Num() <= commitBoxNem)
    {
        arrMenualText.SetNum(commitBoxNem + 1);
    }
    arrMenualText[commitBoxNem] = Text.ToString();

    if (CommitMethod  == ETextCommit::OnEnter)
    {
        int NextBoxNum = BoxNum + 1;
        if(NextBoxNum < arrTextBox.Num())
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                FTimerDelegate::CreateLambda([this, NextBoxNum]()
                    {
                        if (arrTextBox.IsValidIndex(NextBoxNum))
                        {
                            arrTextBox[NextBoxNum]->TextBox->SetKeyboardFocus();
                        }
                    }),
                0.01f,//딜레이 안주면 제대로 안됨
                false
            );
        }
    }
}


void ULineLimitTextBox::NativeConstruct()
{
    if (TextBox)
    {
        TextBox->OnTextChanged.AddDynamic(this, &ULineLimitTextBox::HandleTextChange);
        TextBox->OnTextCommitted.AddDynamic(this, &ULineLimitTextBox::HandleTextCommit);
        TextBoxSize = TextBox->GetCachedGeometry().GetLocalSize();
    }
}


void ULineLimitTextBox::HandleTextChange(const FText& Text)
{
    
    FString CurrentText = Text.ToString();
    FSlateFontInfo FontInfo = TextBox->WidgetStyle.TextStyle.Font;

    TSharedPtr<FSlateFontMeasure>TextMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
    FVector2D TextSize = TextMeasureService->Measure(CurrentText, FontInfo);
    TextBoxSize = TextBox->GetCachedGeometry().GetLocalSize();

    //UE_LOG(LogTemp, Warning, TEXT("잘러잇"));
    if (TextSize.X > TextBoxSize.X || TextSize.Y > TextBoxSize.Y)
    {
        // 글자가 초과되면 잘라내기
        FString Trimmed = CurrentText.LeftChop(1);
        TextBox->SetText(FText::FromString(Trimmed));
    }
    return;
};


void ULineLimitTextBox::HandleTextCommit(const FText& Text, ETextCommit::Type CommitMethod)
{
    MenaulUI->ChangeTextUI(BoxNum,Text, CommitMethod);
}