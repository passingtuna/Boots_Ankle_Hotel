// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Hotel_Types.h"
#include "MenualUI.generated.h"

/**
 * 
 */
class AHotel_Menual;
class AHotel_Walker;
class UHotel_Manager;
class ULineLimitTextBox;
class UVerticalBox;
class UEditableTextBox;

UCLASS()
class BOOTS_ANKLE_HOTEL_API UMenualUI : public UUserWidget
{
	GENERATED_BODY()
	
    AHotel_Menual* aHotelMenual;
    TArray<FManualInfo>* arrMenualInfo;
    TArray<int>* arrExperiencedEventID;

    TArray<FString> arrMenualText;

    UHotel_Manager* Hotel_Manager;
    AHotel_Walker* Hotel_Walker;
    int NowPage;
    int PreMenualLevel;
    bool isGameStarted;
public:
    virtual void NativeConstruct() override;

    void InitMenual(AHotel_Walker* Hotel_Walker);

    UFUNCTION(BlueprintCallable)
    void NextPageButtonAction();

    UFUNCTION(BlueprintCallable)
    void PrevPageButtonAction();

    UFUNCTION(BlueprintCallable)
    void CloseButtonAction();
    
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* LeftPageBox;
    UPROPERTY(meta = (BindWidget))
    UVerticalBox* RightPageBox;
    
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<ULineLimitTextBox> ULineLimitTextBoxClass;

    TArray<ULineLimitTextBox*> arrTextBox;

    bool ShowMenualTextPage();
    void InitMenualText();
    FString MakeFioneerString();


    void SetGameStart() { isGameStarted = true; };

    void ChangeTextUI(int BoxNum , const FText& Text , ETextCommit::Type CommitMethod);
};

UCLASS()
class ULineLimitTextBox : public UUserWidget
{
    GENERATED_BODY()
public:
    FVector2d TextBoxSize;
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* TextBox;
    int BoxNum = -1;
    UMenualUI* MenaulUI;

    UFUNCTION()
    void HandleTextChange(const FText& Text);
    UFUNCTION()
    void HandleTextCommit(const FText& Text, ETextCommit::Type CommitMethod);

protected:
    virtual void NativeConstruct() override;
};