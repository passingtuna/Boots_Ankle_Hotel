// Fill out your copyright notice in the Description page of Project Settings.


#include "PhoneDialUI.h"
#include "Kismet/GameplayStatics.h"
#include "Hotel_Phone.h"
#include "Components/Image.h"

void UPhoneDialUI::PressDial(int Num)
{
    if (!FMath::IsNearlyEqual(DialImage->RenderTransform.Angle, 0, 0.1f)) return;//다이얼이 다시 돌아오는 중이면 버튼 금지
    if (aHotelPhone->aConnectedPhone) return;//전화 받는중 번호 누르기 금지
    if (aHotelPhone->IsDisconnect) return; //한번 연락끊기면 금지
    if (aHotelPhone->NowCallingPhoneNum != "") return;//이미 걸고있는 전화가 있다면

    if (aHotelPhone) aHotelPhone->StopSound(); //다이얼 톤 재생 중지

    GetWorld()->GetTimerManager().ClearTimer(PhoneTimer);
    UGameplayStatics::PlaySound2D(this, arrSoundList[Num - 1]);
    CalRotateImage(Num);
    Num = Num % 10;
    sHoleNumber += FString::FromInt(Num);
}

void UPhoneDialUI::OverInputDelay()
{
    if (aHotelPhone)
    {
        aHotelPhone->TryCalling(FName(*sHoleNumber));
    }
    sHoleNumber = "";
}

void UPhoneDialUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    FWidgetTransform NewTransform = DialImage->RenderTransform;

    // 목표 각도 도달했으면 패스
    if (FMath::IsNearlyEqual(NewTransform.Angle, RotationAngle, 0.1f)) return;

    if (DialImage)
    {
        if (RotationAngle == 0) //원복하기
        {
            NewTransform.Angle -= 250.0f * InDeltaTime; // 회전 각도 (도 단위) 델타 타임으로 틱 보정
            if (NewTransform.Angle <= 0)
            {
                NewTransform.Angle = 0; //원상복구 완료 후 다음 신호 받는 타이머 돌리기
                GetWorld()->GetTimerManager().SetTimer(PhoneTimer, this, &UPhoneDialUI::OverInputDelay, 3.0, false);
            }
            DialImage->SetRenderTransform(NewTransform);
        }
        else //진행
        {
            NewTransform.Angle += 500 * InDeltaTime; // 회전 각도 (도 단위)
            if (NewTransform.Angle >= RotationAngle)
            {
                NewTransform.Angle = RotationAngle;
                GetWorld()->GetTimerManager().SetTimer(DialTimer, this, &UPhoneDialUI::ReturnDial, 0.1, false);
            }
            DialImage->SetRenderTransform(NewTransform);
        }
    }

}
void UPhoneDialUI::ReturnDial() 
{
    RotationAngle = 0; 
};


void UPhoneDialUI::PutDownPhone()
{
    if (aHotelPhone)
    {
        aHotelPhone->EndPhoneUse();
    }
    GetWorld()->GetTimerManager().ClearTimer(DialTimer);
    GetWorld()->GetTimerManager().ClearTimer(PhoneTimer);
    sHoleNumber = "";
    DialImage->RenderTransform.Angle = 0;
    RotationAngle = 0;
    DialImage->SetRenderTransform(DialImage->RenderTransform);
}

void UPhoneDialUI::CalRotateImage(int nNum)
{
    RotationAngle = 60 + (nNum * 25);
}
