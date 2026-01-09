

// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_CCTV.h"
#include "Hotel_Manager.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/TextRenderComponent.h"
#include "Hotel_CCTV_Camera.h"

void AHotel_CCTV::BeginPlay()
{
    ObjectName = TEXT("CCTV 모니터");
    availableAction.Add(TEXT("이전 카메라"), [this]() { ViewPrevCamera(); });
    availableAction.Add(TEXT("다음 카메라"), [this]() { ViewNextCamera(); });
    nNowCameraNum = 0;
    QuickActionName = TEXT("다음 카메라");

    MoniterText = FindComponentByClass<UTextRenderComponent>();
    if (MoniterText)
    {
        MoniterText->SetText(FText::FromString(TEXT("CAMERA ") + arrCameras[nNowCameraNum]->CameraName.ToString()));
    }
    Super::BeginPlay();
    Hotel_Manager->SetHotel_CCTV(this);
}

int AHotel_CCTV::FindNextCameraIndex(bool Next)
{
    int temp = nNowCameraNum;
    int Cal =  (Next == 0) ? -1 : 1; //이전카메라는 -1로 다음카메라는 +1로 탐색
    for (int i = 0; i < arrCameras.Num(); i++) //한바퀴만 돈다
    {
        temp = (temp + Cal + arrCameras.Num()) % arrCameras.Num();
        if (IsValid(arrCameras[temp])) //현재 켜져있는 카메라만 서칭
        {
            FString trigger = "ViewCCTV_" + arrCameras[temp]->CameraName.ToString();
            Hotel_Manager->OnEventTriggerAction(FName(trigger));
            return temp;
        }
    }
    return -1;
}

int AHotel_CCTV::FindCameraNumByName(FName Name)
{
    for (int i = 0; i < arrCameras.Num(); i++)
    {
        if (IsValid(arrCameras[i]))
        {
            if (arrCameras[i]->CameraName == Name)
            {
                return i;
            }
        }
    }
    return -1;
}

void AHotel_CCTV::ViewPrevCamera()
{
    int PrevCameraIndex = FindNextCameraIndex(false);
    if (IsValid(arrCameras[PrevCameraIndex]))
    {
        arrCameras[nNowCameraNum]->SetCameraState(false);
        arrCameras[PrevCameraIndex]->SetCameraState(true);
        nNowCameraNum = PrevCameraIndex;
        MoniterText->SetText(FText::FromString(TEXT("CAMERA ") + arrCameras[nNowCameraNum]->CameraName.ToString()));
    }
    Hotel_Manager->MinusHRScore(60, TEXT("근무 태도 불량"));
}
void AHotel_CCTV::ViewNextCamera()
{
    int NextCameraIndex = FindNextCameraIndex(true);
    if (IsValid(arrCameras[NextCameraIndex]))
    {
        arrCameras[nNowCameraNum]->SetCameraState(false);
        arrCameras[NextCameraIndex]->SetCameraState(true);
        nNowCameraNum = NextCameraIndex;
        MoniterText->SetText(FText::FromString(TEXT("CAMERA ") + arrCameras[nNowCameraNum]->CameraName.ToString()));
    }

    Hotel_Manager->UpdateDefualtLevelMenual(NextCameraIndex);
}

void AHotel_CCTV::ViewIndexCamera(int Index)
{
    if (arrCameras.IsValidIndex(Index))
    {
        int NextCameraIndex = Index;
        if (IsValid(arrCameras[NextCameraIndex]))
        {
            arrCameras[nNowCameraNum]->SetCameraState(false);
            arrCameras[NextCameraIndex]->SetCameraState(true);
            nNowCameraNum = NextCameraIndex;
            MoniterText->SetText(FText::FromString(TEXT("CAMERA ") + arrCameras[nNowCameraNum]->CameraName.ToString()));
        }
    }
}

void AHotel_CCTV::SetPostProcessCamera(int CameraNum, int ProcessingNum , bool On)
{
    if (CameraNum >= arrCameras.Num()) return;
    arrCameras[CameraNum]->SetPostProcess(ProcessingNum , On);
}

void AHotel_CCTV::SetHiddenObject(AActor * HiddenActor)
{
    for (const auto& Elem : arrCameras)
    {
        if (!Elem)return;
        USceneCaptureComponent2D* CaptureComp = Elem->CameraComp;
        if(CaptureComp) CaptureComp->HiddenActors.Add(HiddenActor);
    }
}

int AHotel_CCTV::GetRoomCameraIndex(FName cameraname)
{
    if (cameraname == "201")      return 4;
    else if (cameraname == "304") return 13;
    else if (cameraname == "202") return 5;
    else if (cameraname == "203") return 6;
    else if (cameraname == "204") return 7;
    else if (cameraname == "301") return 10;
    else if (cameraname == "302") return 11;
    else if (cameraname == "303") return 12;
    return -1;
}

void AHotel_CCTV::ActiveRoomCameraToCCTV(AHotel_CCTV_Camera* camera) 
{ 
    if (!IsValid(camera)) return;
    int CameraIndex = GetRoomCameraIndex(camera->CameraName);

    if (CameraIndex > 3)
    {
        arrCameras[CameraIndex] = camera;
        arrCameras[CameraIndex]->SetCameraState(false);
    }
};
void AHotel_CCTV::EraseRoomCameraToCCTV(AHotel_CCTV_Camera* camera)
{
    if (!IsValid(camera)) return;
    int CameraIndex = GetRoomCameraIndex(camera->CameraName);
    if (CameraIndex > 3)
    {
        arrCameras[CameraIndex] = NULL;
    }
};
