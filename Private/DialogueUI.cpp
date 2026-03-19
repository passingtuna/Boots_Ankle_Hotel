// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueUI.h"
#include "Hotel_Walker.h"
#include "AI_Hotel_Guest_Default.h"
#include "Hotel_Manager.h"
#include "Hotel_Guest_Room.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "DialogueDataAsset.h"
#include "Hotel_Guest.h"
#include "Hotel_Phone.h"

void UDialogueUI::NativeConstruct()
{
    for (int i = 0 ; i < 10 ; i++)
    {
        if (UDialogueChoiceButtonClass)
        {
            UDialogueChoiceButton* ChoiceButton = CreateWidget<UDialogueChoiceButton>(this, UDialogueChoiceButtonClass);
            ChoiceButton->TextBlock->Font.Size = 16;
            ChoiceButton->Button->IsFocusable = true;
            ChoiceButton->SetVisibility(ESlateVisibility::Collapsed);
            ChoiceButton->DialogueWidget = this;
            ChooseButtonContainer->AddChild(ChoiceButton);
            arrChoiceButton.Add(ChoiceButton);
        }
    }

    UDialogueChoiceButton* tempWaitButton = CreateWidget<UDialogueChoiceButton>(this, UDialogueChoiceButtonClass);
    tempWaitButton->TextBlock->Font.Size = 16;
    tempWaitButton->Button->IsFocusable = true;
    tempWaitButton->DialogueWidget = this;
    tempWaitButton->TextBlock->SetText(FText::FromString(TEXT("잠시 기다려 주실수 있을실까요?")));
    tempWaitButton->NextDialogueId = 0;
    tempWaitButton->SetVisibility(ESlateVisibility::Visible);
    tempWaitButton->StoredAction = [this]() { PauseDialogue(); };
    ChooseButtonContainer->AddChild(tempWaitButton);
    WaitButton = tempWaitButton;

    UDialogueChoiceButton* tempEndButton = CreateWidget<UDialogueChoiceButton>(this, UDialogueChoiceButtonClass);
    tempEndButton->TextBlock->Font.Size = 16;
    tempEndButton->Button->IsFocusable = true;
    tempEndButton->DialogueWidget = this;
    tempEndButton->TextBlock->SetText(FText::FromString(TEXT("나중에 하자")));
    tempEndButton->NextDialogueId = 0;
    tempEndButton->SetVisibility(ESlateVisibility::Collapsed);
    tempEndButton->StoredAction = [this]() { EndDialogue(); };
    ChooseButtonContainer->AddChild(tempEndButton);
    EndButton = tempEndButton;

    mapFunction.Add(TEXT("End"), [this]() { EndDialogue(); });
    mapFunction.Add(TEXT("RejectFullRoom"), [this]() { RejectFullRooms(); });
    mapFunction.Add(TEXT("CheckIn"), [this]() { CheckIn(); });
    mapFunction.Add(TEXT("UpdateGuestName"), [this]() { UpdateGuestName(); });
    mapFunction.Add(TEXT("OpenDoor"), [this]() { OpenDoor(); });
    mapFunction.Add(TEXT("Reporting"), [this]() { Reporting(); });
    mapFunction.Add(TEXT("SecurityReport"), [this]() { SecurityReport(); });
    mapFunction.Add(TEXT("DisconnectCalling"), [this]() { DisconnectCalling(); });
    mapFunction.Add(TEXT("SetDialogueIndex"), [this]() { SetDialogueIndex(); });
    mapFunction.Add(TEXT("MinusHRResource"), [this]() { MinusHRResource(); });
    mapFunction.Add(TEXT("FireWalker"), [this]() { FireWalker(); });
    mapFunction.Add(TEXT("DecereasePatience"), [this]() { DecereasePatience(); });
    mapFunction.Add(TEXT("CheckRoomCondition"), [this]() { CheckRoomCondition(); });
    mapFunction.Add(TEXT("ApologizeAccept"), [this]() { ApologizeAccept(); });

    
        

    
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
}
void UDialogueUI::ViewSelectionDialogue()
{
    isAlreadyEnd = false;
    isPrevDisConnect = false;
    TextNameBox->SetText(FText::FromString(""));
    FText FormatNamedLine = FText::FromString(TEXT("(어떤 대화를 시작할까?)"));
    TextLineBox->SetText(FormatNamedLine);
    TextNameBox->SetText(FText::FromString(TEXT("근무자")));


    if (!ChooseButtonContainer) return;
    DeactivateAllButton();
    WaitButton->SetVisibility(ESlateVisibility::Collapsed);
    EndButton->SetVisibility(ESlateVisibility::Visible);
    TArray<FDialogueChoice> arrTitleButton;
    FDialogueChoice tempButton;
    for (int i = 1 ; i < NowGuest->arrGuestDialogueData.Num() ; ++i)
    {
        tempButton.FunctionName = "SetDialogueIndex";
        tempButton.Text = NowGuest->arrGuestDialogueData[i].DialogueTitle;
        tempButton.FunctionParameter = i;
        tempButton.NextLineId = 0;
        arrTitleButton.Add(tempButton);
    }

    for (const auto& Elem : arrTitleButton)
    {
        ActivateButton(Elem);
    }
}

void UDialogueUI::ViewDialogue()
{
    if (!IsValid(NowDialogueData))
    {
        return;
    }
    isPrevDisConnect = false;
    isAlreadyEnd = false;
    FText FormatNamedLine = GetFormatNamedText(NowDialogueData->DialogueLines[NowDialogueIndex].Line);
    TextLineBox->SetText(FormatNamedLine);

    if (NowDialogueData->DialogueLines[NowDialogueIndex].isPausable)
    {
        WaitButton->SetVisibility(ESlateVisibility::Visible);
    }
    else
    {
        WaitButton->SetVisibility(ESlateVisibility::Collapsed);
    }
    EndButton->SetVisibility(ESlateVisibility::Collapsed);
    if (IsValid(OverlayPhone))//폰으로 받은 전화는 이름표시 X
    {
        TextNameBox->SetText(FText::FromString("????"));
    }
    else
    {
        TextNameBox->SetText(FText::FromString(DialogueName));
    }
    if (!ChooseButtonContainer) return;
    DeactivateAllButton();
    for (const auto& Elem : NowDialogueData->DialogueLines[NowDialogueIndex].ChoicesButton)
    {
        ActivateButton(Elem);
    }
}


void UDialogueUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
}

void UDialogueUI::SetDialogueGuest(AHotel_Guest* GuestInfo, AHotel_Phone* Phone)
{
    NowGuest = GuestInfo;
    OverlayPhone = NULL;
    NowGuestDialogueIndex = -1;
    NowDialogueIndex = -1;
    NowDialogueData = NULL;

    NowGuestController = Cast<AAI_Hotel_Guest_Default>(NowGuest->GetController());
    if (NowGuestController)
    {
        NowGuestController->StopPatienceTimer();
    }

    GetWorld()->GetTimerManager().ClearTimer(DialogueTimer);

    DialogueName = NowGuest->ObjectName;
    if (IsValid(Phone))
    {
        OverlayPhone = Phone;
    }
    else //전화로 응대한게 아니라면
    {
        if (IsValid(NowGuest) && !NowGuest->GetIsLookingFace()) // 손님 얼굴 보지않고 응대시
        {
            if (!NowGuest->IsWierdFaceLook)//이상 손님이 아니라면
            {
                Hotel_Manager->MinusHRScore(5,TEXT("불친절한 응대"));
                //근무점수 감점
            }
        }
    }

    if (NowGuest->NowDialogueData)
    {
        NowDialogueData = NowGuest->NowDialogueData;
        NowGuestDialogueIndex = NowGuest->LastGuestDialogueDataIndex;
        if (NowGuest->arrGuestDialogueData.Num() > NowGuestDialogueIndex)
        {
            NowDialogueIndex = NowGuest->arrGuestDialogueData[NowGuestDialogueIndex].DialogueIndex;
        }
        else
        {
            NowDialogueIndex = 0;
        }
        ViewDialogue();
    }
    else if (IsValid(OverlayPhone) || (NowGuest->arrGuestDialogueData.Num() != 0 && NowGuest->arrGuestDialogueData.Num() <= 2))
    {
        NowDialogueData = NowGuest->GetDailogueData(NowGuest->arrGuestDialogueData.Last().DialogueState, IsValid(Phone));
        NowDialogueIndex = NowGuest->arrGuestDialogueData.Last().DialogueIndex;
        NowGuestDialogueIndex = NowGuest->arrGuestDialogueData.Num()-1;
        NowGuest->NowDialogueData = NowDialogueData;
        ViewDialogue();
    }
    else if (NowGuest->arrGuestDialogueData.Num() > 2)
    {
        ViewSelectionDialogue(); //선택대화 아무것도 없다면 기본 대화
    }
}


FText UDialogueUI::GetFormatNamedText(const FString& Text)
{
    FFormatNamedArguments Args;

    if (IsValid(NowGuest))
    {
        Args.Add("GuestName", FText::FromString(NowGuest->ObjectName));
        if (IsValid(NowGuestController) && IsValid(NowGuestController->AssignedGuestRoom)) Args.Add("RoomNumber", FText::FromName(NowGuestController->AssignedGuestRoom->GetRoomNumber()));
        if (IsValid(Hotel_Manager))
        {
            Args.Add("CodeWord", FText::FromString(Hotel_Manager->GetCodeWord()));
            Args.Add("WrongCodeWord", FText::FromString(Hotel_Manager->GetWrongCodeWord()));
            FString tempString;
            tempString = "AllocateRoom";
            if (Text.Contains("{" + tempString + "}"))
            {
                Args.Add(tempString, FText::FromString(Hotel_Manager->GetAllocateRoomNum()));
            }
        }
    }


    FText tempText= FText::FromString(Text.ReplaceEscapedCharWithChar());
    
    FText FinalLine = FText::Format(tempText, Args);
    return FinalLine;
}

void UDialogueUI::PauseDialogue()
{
    if (isPrevDisConnect)
    {
        EndDialogue();
    }

    if (IsValid(NowGuest))
    {
        NowGuest->SetGuestDialogueData(NowDialogueData, NowGuestDialogueIndex);
        NowGuest->SetDialoguePause(NowGuestDialogueIndex , NowDialogueIndex);

        GetWorld()->GetTimerManager().SetTimer(DialogueTimer, [this]() {NowGuest->GetAIController()->DecreasePatienceCount(true); }, 10 , false);
    }
    Hotel_Walker->HideUIName("Dialogue");

    if (IsValid(OverlayPhone))
    {
        Hotel_Walker->HideUIName("Phone");
    }
    if (!isPrevDisConnect) Hotel_Walker->GetMouseControl(); //상대방이 끊은게 아니라면마우스 컨트롤 넘기지 말기


}
void UDialogueUI::GuestWaitOvertime()
{
    //대화를 너무 길게 멈추면 패널티
    Hotel_Manager->MinusHRScore(10, TEXT("고객 응대 지연"));
    EndDialogue();
}

void UDialogueUI::EndDialogue()
{
    NowGuest->EndGuestDialgue(NowGuestDialogueIndex);
    Hotel_Walker->HideUIName("Dialogue");
    if (isPrevDisConnect || isAlreadyEnd) return; 
    isAlreadyEnd = true;
    if (IsValid(OverlayPhone))
    {
        OverlayPhone->aConnectedPhone->Disconnect();
        OverlayPhone->Disconnect();
        OverlayPhone = NULL;
    }
    else
    {
        Hotel_Walker->GetMouseControl();
    }
}

void UDialogueUI::ActivateButton(const FDialogueChoice& ChoiceData)
{
    for (int i = 0 ; i < arrChoiceButton.Num();i++)
    {
        if (arrChoiceButton[i]->GetVisibility() == ESlateVisibility::Visible) continue; //이미 활성화 되었다면 넘기기
        arrChoiceButton[i]->SetVisibility(ESlateVisibility::Visible);
        FText FormatNamedLine = GetFormatNamedText(ChoiceData.Text);
        arrChoiceButton[i]->TextBlock->SetText(FormatNamedLine);
        arrChoiceButton[i]->NextDialogueId = ChoiceData.NextLineId;
        arrChoiceButton[i]->FunctionParameter = ChoiceData.FunctionParameter;
        if(mapFunction.Contains(ChoiceData.FunctionName))
        {
            arrChoiceButton[i]->StoredAction = *mapFunction.Find(ChoiceData.FunctionName);
        }
        break;
    }
}
void UDialogueUI::DeactivateAllButton()
{
    for (int i = 0; i < arrChoiceButton.Num(); i++)
    {
        arrChoiceButton[i]->SetVisibility(ESlateVisibility::Collapsed);
        arrChoiceButton[i]->TextBlock->SetText(FText::FromString(TEXT("")));
        arrChoiceButton[i]->NextDialogueId = 0;
        arrChoiceButton[i]->FunctionParameter = 0;
        arrChoiceButton[i]->StoredAction = NULL;
    }
}


void UDialogueUI::SetNowIndex(int index)
{
    if (isAlreadyEnd) return; //EndDialogue 함수로 이미 대화가 끝났다면
    if (isPreInitDialogueIndex)
    {
        isPreInitDialogueIndex = false;
        return;
    }

    bool isFindIndex = false;

    for (int i = 0; i < NowDialogueData->DialogueLines.Num(); i++)
    {
        if (NowDialogueData->DialogueLines[i].NodeID == index)
        {
            NowDialogueIndex = index;
            isFindIndex = true;
        }
    }
    if (isFindIndex)
    {
        ViewDialogue();
    }
    else
    {
        EndDialogue();
    }
}

void UDialogueUI::UpdateGuestName()
{
    if (IsValid(NowGuest))
    {
        NowGuest->UpdateGuestName();
        DialogueName = NowGuest->ObjectName;
    }
}
void UDialogueUI::RejectFullRooms()
{
    if (IsValid(NowGuest))
    {
        if (NowGuest->IsCheckTrigger)
        {
            Hotel_Manager->OnEventTriggerAction(
                FHotelTrigger::Make(EHotelTriggerType::GuestOutHotel,
                    {
                        { EHotelTriggerKey::Instigator, NowGuest->GuestName },
                    }));
        }
        NowGuest->GuestExit();
    }
    EndDialogue();
}

void UDialogueUI::CheckIn()
{
    FName temp = FName(*FString::FromInt(NowFunctionParameter));
    if (Hotel_Manager->CheckGuestRoom(temp)) //방에 이상없으면 체크인 하고 행동
    {
        Hotel_Manager->CheckInGuestRoom(NowGuest, temp);
        if (NowGuest->IsCheckTrigger)
        {
            Hotel_Manager->OnEventTriggerAction(
                FHotelTrigger::Make(EHotelTriggerType::GuestCheckIn,
                    {
                        { EHotelTriggerKey::Instigator, NowGuest->GuestName },
                        { EHotelTriggerKey::RoomNumber, temp.ToString() }
                    }));
        }
        EndDialogue();
    }
    else //방에 이상이 있다면 현재 다이얼로그 재실행
    {
        SetNextDialogueIndex(NowDialogueIndex);
        ViewDialogue();
        TextLineBox->SetText(FText::FromString(TEXT("장난 하시는겁니까?")));
        if (NowGuest && NowGuest->AIController)
        {
            NowGuest->AIController->DecreasePatienceCount(false);
            if (NowGuest->AIController->GetPatienceCount() < 1)
            {
                EndDialogue();
            }
        }
    }
}


void UDialogueUI::ExecuteTimerFunction()
{
    if (TimerFuntion)
    {
        TimerFuntion();
    }
}

void UDialogueUI::OpenDoor()
{
    if (IsValid(NowGuest))
    {
        NowGuest->ReleaseDoorLock();
    }
    EndDialogue();
}

void UDialogueUI::Reporting()
{
    FName temp = FName(*FString::FromInt(NowFunctionParameter));
    if (Hotel_Manager->CheckGuestRoom(temp)) //방에 이상없으면 체크인 하고 행동 / 방에 이상이 있다면 다음 다이얼로그 실행
    {
        Hotel_Manager->CheckInGuestRoom(NowGuest, temp);
        EndDialogue();
    }
}

void UDialogueUI::SetNextDialogueIndex(int index)
{
    isPreInitDialogueIndex = true;

    if (!IsValid(NowDialogueData))
    {
        return;
    }
    for (int i = 0 ; i < NowDialogueData->DialogueLines.Num() ;i++)
    {
        if (NowDialogueData->DialogueLines[i].NodeID == index)
        {
            NowDialogueIndex = index;
            return;
        }
    }
}

void UDialogueUI::SecurityReport()
{
    const FString ReportTarget = (NowFunctionParameter == 0)
        ? TEXT("Lobby")
        : FString::FromInt(NowFunctionParameter);

    if (Hotel_Manager->OnEventTriggerAction(FHotelTrigger::Make(EHotelTriggerType::SecurityReport,
        { { EHotelTriggerKey::Target, ReportTarget } })))   //발동된 트리거가 있을시 정확한 신고 
    {
        SetNextDialogueIndex(1);
        DisconnectCalling();
        ViewDialogue();
    }
    else //아닐시 패널티
    {
        SetNextDialogueIndex(2);
        DisconnectCalling();
        ViewDialogue();
        Hotel_Manager->MinusHRScore(10, TEXT("보안팀 관련 보고 부정확"));//인사 점수
    }
}

void UDialogueUI::DisconnectCalling()
{
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            if (IsValid(OverlayPhone))
            {
                OverlayPhone->aConnectedPhone->Disconnect();
                OverlayPhone->Disconnect();
                OverlayPhone = NULL;
            }
            isPrevDisConnect = true;

        },1.0f, false); //말은 하고 끊는다는 느낌으로 1초뒤에 전화 끊김 사운드
}

void UDialogueUI::SetDialogueIndex()
{
    if (NowGuest->arrGuestDialogueData.Num() > NowFunctionParameter)
    {
        NowGuestDialogueIndex = NowFunctionParameter;
        NowDialogueData = NowGuest->GetDailogueData(NowGuest->arrGuestDialogueData[NowGuestDialogueIndex].DialogueState, IsValid(OverlayPhone));
        SetNextDialogueIndex(NowGuest->arrGuestDialogueData[NowGuestDialogueIndex].DialogueIndex);
        NowGuest->SetGuestDialogueData(NowDialogueData, NowGuestDialogueIndex);
        ViewDialogue();
    }
}

void UDialogueUI::MinusHRResource()
{
    switch (NowFunctionParameter)
    {
    case 0: Hotel_Manager->MinusHRScore(30, TEXT("대화중 매니저 노출")); break;
    case 1:
        {
            Hotel_Manager->MinusHRScore(20, TEXT("손님 조롱"));
            if (IsValid(NowGuest))
            {
                NowGuest->GuestExit();
            }
        }break;
    case 2:
        Hotel_Manager->MinusHRScore(20, TEXT("손님 조롱"));
        EndDialogue();
        break;
    }
}

void UDialogueUI::FireWalker()
{
    //DisconnectCalling();
    //EndDialogue();
    Hotel_Manager->SetGameEnd(GER_Fired);
}

void UDialogueUI::DecereasePatience()
{
    if (IsValid(NowGuest))
    {
        NowGuest->AIController->DecreasePatienceCount(false);
    }
}
void UDialogueUI::CheckRoomCondition()
{
    if (IsValid(NowGuest))
    {
        if (NowGuest->AIController->AssignedGuestRoom->CheckRoomDirty()) 
        {
            NowGuest->EraseDialgueDataState(EDialogueState::DS_Guest_Complete_RoomClean);
            EndDialogue();
        }
    }
}

void UDialogueUI::ApologizeAccept()
{
    NowGuest->EraseDialgueDataState(EDialogueState::DS_Guest_Apologize);
}