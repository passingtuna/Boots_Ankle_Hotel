// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhoneDialUI.generated.h"

class AHotel_Phone;
class UImage;
/**
 * 
 */
UCLASS(Blueprintable)
class BOOTS_ANKLE_HOTEL_API UPhoneDialUI : public UUserWidget
{
	GENERATED_BODY()

private:
    FString sHoleNumber;
    FTimerHandle PhoneTimer;
    FTimerHandle DialTimer;
    bool TryCalling;
public:

    UFUNCTION(BlueprintCallable)
    void PressDial(int Num);
    UFUNCTION(BlueprintCallable)
    void PutDownPhone();

    UFUNCTION()
    void OverInputDelay();
    AHotel_Phone* aHotelPhone;

    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    void SetPhoneObject(AHotel_Phone* PhonObject)
    {
        aHotelPhone = PhonObject;
    };
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    TArray<USoundBase*> arrSoundList;

    UPROPERTY(meta = (BindWidget))
    UImage* DialImage;
    void CalRotateImage(int nNum);

    UFUNCTION()
    void ReturnDial();
    float RotationAngle = 0;

};
