// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hotel_Object.h"
#include "Hotel_Phone.generated.h"

class UPhoneDialUI;
class AHotel_Guest;
/**
 * 
 */
UCLASS(Blueprintable)
class BOOTS_ANKLE_HOTEL_API AHotel_Phone : public AHotel_Object
{
	GENERATED_BODY()
	
private:
    virtual void BeginPlay() override;
    bool IsRinging;

public:
    UPROPERTY(EditAnywhere ,BlueprintReadWrite,Category = "Phone"); //이 전화기의 전화번호
    FName RegistPhoneNumber;

    FName NowCallingPhoneNum;           //현재 걸고 있는 전화번호

    AHotel_Phone* aConnectedPhone;
    bool isUserPickUpPhone;
    FTimerHandle PhoneTimer;
    bool IsDisconnect;
    void TryCalling(FName PhoneNum);
    void EndPhoneUse();

    UFUNCTION()
    void PlayToneDial();

    UFUNCTION()
    void ConnectFail();

    UFUNCTION()
    void ConnectTry(AHotel_Phone* ConnectingPhone);
    void Disconnect();

    UFUNCTION()
    void ConnectSuccess();
    void RecieveCalling();
    void PickReceiver();
    void RingingTimeOver();
    FName GetRegistPhoneNumber() {return RegistPhoneNumber;};

    void InitPhoneWatchGuest();

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Sound)
    AHotel_Guest* PhoneWatchGuest;
    void SetPhoneNumber(FName PhoneNum);
    void SetPhoneWatchGuest(AHotel_Guest* Guest) { PhoneWatchGuest = Guest; };
};
