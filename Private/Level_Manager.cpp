// Fill out your copyright notice in the Description page of Project Settings.


#include "Level_Manager.h"
#include "Hotel_Manager.h"
#include "GameFramework/Controller.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
ALevel_Manager::ALevel_Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true; 

    AudioCompBGM = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMAudio"));
    AudioCompRinging = CreateDefaultSubobject<UAudioComponent>(TEXT("RingAudio"));
}

// Called when the game starts or when spawned
void ALevel_Manager::BeginPlay()
{
	Super::BeginPlay();

    PlayerController = GetWorld()->GetFirstPlayerController();
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();

    if (MainMenuUiWidgetClass)
    {
        MainMenuUI = CreateWidget<UMainMenuUI>(PlayerController, MainMenuUiWidgetClass);
        if (MainMenuUI)
        {
            MainMenuUI->AddToViewport();
            MainMenuUI->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (LoadingUIWidgetClass)
    {
        LoadingUI = CreateWidget<ULoadingUI>(PlayerController, LoadingUIWidgetClass);
        if (LoadingUI)
        {
            LoadingUI->AddToViewport();
            LoadingUI->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (EndingUIWidgetClass)
    {
        EndingUI = CreateWidget<UGameEndReportUI>(PlayerController, EndingUIWidgetClass);
        if (EndingUI)
        {
            EndingUI->AddToViewport();
            EndingUI->SetVisibility(ESlateVisibility::Hidden);
        }
    }

        
    Hotel_Manager->SetLevelManager(this);
    UGameplayStatics::LoadStreamLevel(this, FName("Level_Outside"), true, true, FLatentActionInfo());
    FLatentActionInfo MainLatenActionInfo;
    MainLatenActionInfo.CallbackTarget = this;
    MainLatenActionInfo.ExecutionFunction = "LoadMainComplete";
    MainLatenActionInfo.UUID = 0;
    MainLatenActionInfo.Linkage = 0;
    UGameplayStatics::UnloadStreamLevel(this, FName("Level_End"), FLatentActionInfo(), true);
    UGameplayStatics::LoadStreamLevel(this, FName("Level_Main"), true, true, MainLatenActionInfo);
    /*
    if (AudioCompBGM)
    {
        AudioCompBGM->Play();
    }*/

}
void ALevel_Manager::LoadMainLevel()
{
    UGameplayStatics::OpenLevel(this, FName("Level_Persistant"));

}
void ALevel_Manager::LoadEndingLevel()
{

    ViewLevelUI(1);
    UGameplayStatics::UnloadStreamLevel(this, FName("Level_Main"), FLatentActionInfo(), true);
    FLatentActionInfo EndingLatenActionInfo;
    EndingLatenActionInfo.CallbackTarget = this;
    EndingLatenActionInfo.ExecutionFunction = "LoadEndingComplete";
    EndingLatenActionInfo.UUID = 1;
    EndingLatenActionInfo.Linkage = 0;
    UGameplayStatics::LoadStreamLevel(this, "Level_End", true, true, EndingLatenActionInfo);
}

void ALevel_Manager::LoadMainComplete()
{
    DirectionalLight->SetBrightness(0);
    HideLevelUI(1);
    HideLevelUI(2);
    ViewLevelUI(0);
    Hotel_Manager->CompleteMainLevelLoad();
}
void ALevel_Manager::LoadEndingComplete()
{
    HideLevelUI(1);
    ViewLevelUI(2);

    if (AudioCompBGM)
    {
        AudioCompBGM->Stop();
    }
}
void ALevel_Manager::ViewLevelUI(int uIType)
{
    AHotel_Walker* Hotel_Walker = Hotel_Manager->GetHotelWalker();

    switch(uIType)
    {
    case 0:
        {
            if (MainMenuUI)
            {
                if (Hotel_Walker)
                {
                    MainMenuUI->SetHotelWalker(Hotel_Walker);
                    Hotel_Walker->GiveMouseControl(MainMenuUI);
                }
                MainMenuUI->SetVisibility(ESlateVisibility::Visible);
            }
        }
        break;
    case 1:
        {
            if (LoadingUI)
            {
                if (Hotel_Walker)
                {
                    Hotel_Walker->GiveMouseControl(LoadingUI);
                }
                LoadingUI->SetVisibility(ESlateVisibility::Visible);
            }
        }
        break;
    case 2:
        if (EndingUI)
        {
            if (Hotel_Walker)
            {
                Hotel_Walker->GiveMouseControl(EndingUI);
            }
            EndingUI->SetHRReasonText();
            EndingUI->SetVisibility(ESlateVisibility::Visible);
        }
        break;
    }
}

void ALevel_Manager::HideLevelUI(int uiType)
{
    switch (uiType)
    {
    case 0:
        if (MainMenuUI)
        {
            MainMenuUI->SetVisibility(ESlateVisibility::Hidden);
        }
        break;
    case 1:
        if (LoadingUI)
        {
            LoadingUI->SetVisibility(ESlateVisibility::Hidden);
        }
        break;
    case 2:
        if (EndingUI)
        {
            EndingUI->SetVisibility(ESlateVisibility::Hidden);
        }
        break;
    }
}
void ALevel_Manager::RingingBell()
{
    if (AudioCompRinging)
    {
        AudioCompRinging->SetSound(RingSoundEffect);
        AudioCompRinging->Play();
    }
}