// Fill out your copyright notice in the Description page of Project Settings.


#include "OptionMenuUI.h"
#include "Hotel_Walker.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h" 
#include "HotelSaveGame.h"
#include "Hotel_Manager.h"


void UOptionMenuUI::NativeConstruct()
{
    UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/BluePrint/Data/Table/DT_Menual"));
    arrMenualInfo.Empty();
    if (Table)
    {
        static const FString Context(TEXT("DT_Menual"));
        TArray<FManualInfo*> TempRow;
        Table->GetAllRows<FManualInfo>(Context, TempRow);
        for (auto& Row : TempRow)
        {
            arrMenualInfo.Add(*Row);
        }
    }
}
void UOptionMenuUI::InitOption()
{
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    MenualLevel = Hotel_Manager->GetMenualLevel();
    EnviromentLevel = Hotel_Manager->GetEnviromentalLevel();
    LoadedMenualLevel = MenualLevel;

    HighLightButtonImage(true , MenualLevel);
    HighLightButtonImage(false, EnviromentLevel);
    ConfirmPanel->SetVisibility(ESlateVisibility::Collapsed);
}

void UOptionMenuUI::HighLightButtonImage(bool isMenaul, int level)
{
    if (isMenaul)
    {
        MenualLevelImage0->SetRenderOpacity(0.0f);
        MenualLevelImage1->SetRenderOpacity(0.0f);
        MenualLevelImage2->SetRenderOpacity(0.0f);
        switch (level)
        {
        case 0:
            MenualLevelImage0->SetRenderOpacity(1.0f);
            break;
        case 1:
            MenualLevelImage1->SetRenderOpacity(1.0f);
            break;
        case 2:
            MenualLevelImage2->SetRenderOpacity(1.0f);
            break;
        }
    }
    else
    {
        EnviromentLevelImage0->SetRenderOpacity(0.0f);
        EnviromentLevelImage1->SetRenderOpacity(0.0f);
        EnviromentLevelImage2->SetRenderOpacity(0.0f);
        switch (level)
        {
        case 0:
            EnviromentLevelImage0->SetRenderOpacity(1.0f);
            break;
        case 1:
            EnviromentLevelImage1->SetRenderOpacity(1.0f);
            break;
        case 2:
            EnviromentLevelImage2->SetRenderOpacity(1.0f);
            break;
        }
    }
}
void UOptionMenuUI::SaveButtonAction()
{

    Hotel_Manager->SaveLevelOption(MenualLevel,EnviromentLevel);

    if (MenualLevel != LoadedMenualLevel)//메뉴얼 난이도가 변경되었다면
    {
        Hotel_Manager->InitMenualInfo();
        if (MenualLevel == 2)
        {
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            // 폴더가 없으면 생성
            if (!PlatformFile.DirectoryExists(*FPaths::ProjectSavedDir()))
            {
                PlatformFile.CreateDirectoryTree(*FPaths::ProjectSavedDir());
            }
        }
    }

    SetVisibility(ESlateVisibility::Collapsed);
}
void UOptionMenuUI::ExitButtonAction()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

void UOptionMenuUI::EnviromentLevelButtonAction(int Level)
{
    HighLightButtonImage(false , Level);
   EnviromentLevel = Level;
}
void UOptionMenuUI::MenualLevelButtonAction(int Level)
{
    HighLightButtonImage(true, Level);
    if (MenualLevel != Level)//메뉴얼 난이도가 변경되었다면
    {
        ConfirmPanel->SetVisibility(ESlateVisibility::Visible);
    }
    MenualLevel = Level;
}


void UOptionMenuUI::CloseButtonAction()
{
    ConfirmPanel->SetVisibility(ESlateVisibility::Collapsed);
}