// Fill out your copyright notice in the Description page of Project Settings.


#include "Hotel_Guest.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/UObjectGlobals.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Hotel_Walker.h"
#include "Hotel_Door.h"
#include "AI_Hotel_Guest_Default.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interactable_Object.h"
#include "DialogueDataAsset.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "GuestAnimInstance.h"
#include "GuestFaceAnimInstance.h"
#include "Hotel_CCTV_Camera.h"
#include "Hotel_Manager.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
AHotel_Guest::AHotel_Guest()
{
    ObjectName = TEXT("????");
    PrimaryActorTick.bCanEverTick = true;
    availableAction.Add(TEXT("대화"), [this]() {OpenConversationUI(); });
    QuickActionName = TEXT("대화");
    AIControllerClass = AAI_Hotel_Guest_Default::StaticClass();

    AudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
    IsAutoActionDoor = true;
    IsCalledWalker = false;
    IsLookingPlayer = false;
    bWasLookingAtGuest = false;
    GuestName = "";
    IsMan = false;
    IsAlreadyNeckShaking = false;
    IsLookingFace = false;
    IsCheckTrigger = false;
    AddDialogueDataState(TEXT("인사"), EDialogueState::DS_Guest_Default, 0);

}

// Called when the game starts or when spawned
void AHotel_Guest::BeginPlay()
{
    Super::BeginPlay();
    Hotel_Walker = Cast<AHotel_Walker>(GetWorld()->GetFirstPlayerController()->GetPawn());

    FString TempString;
    if (availableAction.Num() > 0)
    {
        TempString += TEXT("[우클릭] 상호작용 메뉴 : ") + ObjectName;
    }
    if (QuickActionName != "없음")
    {
        TempString += TEXT("\n[E] : 퀵 액션 : ") + QuickActionName.ToString();
    }
    InteractMassage = TempString;

    GetComponents<USkeletalMeshComponent>(MeshComponents);

    if (!MeshComponents.IsEmpty())
    {
        for (USkeletalMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp)
            {
                MeshComp->SetRenderCustomDepth(true);
                if (MeshComp->GetName() == "CharacterMesh0")
                {
                    BedyAnimInstance = Cast<UGuestAnimInstance>(MeshComp->GetAnimInstance());
                }
                if (MeshComp->GetName() == "Face")
                {
                    FaceAnimInstance = Cast<UGuestFaceAnimInstance>(MeshComp->GetAnimInstance());
                }
            }
        }
    }
    Hotel_Manager = GetWorld()->GetGameInstance()->GetSubsystem<UHotel_Manager>();
    FaceCollision = FindComponentByClass<USphereComponent>();
    CapsuleComponent = FindComponentByClass<UCapsuleComponent>();
    IsHanging = false;
    IsReadyToNeckShaking = false;
    GetCharacterMovement()->MaxWalkSpeed = 200;
}


void AHotel_Guest::UpdateInteractMessage()
{
    FString TempString;
    if (availableAction.Num() > 0)
    {
        TempString += TEXT("[우클릭] 상호작용 메뉴 : ") + ObjectName;
    }
    if (QuickActionName != "없음")
    {
        TempString += TEXT("\n[E] : 퀵 액션 : ") + QuickActionName.ToString();
    }
    InteractMassage = TempString;
}
void AHotel_Guest::UpdateGuestName() 
{ 
    ObjectName = GuestName; 
    UpdateInteractMessage();
}

// Called every frame
void AHotel_Guest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (Hotel_Walker && IsLookingPlayer)
    {
        SetLookActor(Hotel_Walker);
    }

    if (Hotel_Walker && IsReadyToNeckShaking && !IsAlreadyNeckShaking)
    {
        NeckShakeCheckElapsed += DeltaTime;
        // 목 흔들기/캐치 체크는 너무 자주 할 필요가 없으므로 약간의 주기를 둔다.
        if (NeckShakeCheckElapsed < 0.05f) // 20fps 수준으로 제한
        {
            return;
        }
        NeckShakeCheckElapsed = 0.0f;

        float Distance = FVector::Dist(GetActorLocation(), Hotel_Walker->GetActorLocation());
        if (Distance < 200)
        {
            CatchingPlayer();
            if (IsWierdHanging) Hotel_Manager->UpdateDefualtLevelMenual(5);
        }
    }

    if (Hotel_Walker && Hotel_Manager && IsWierdStareUnderLight)
    {
        WeirdStareCheckElapsed += DeltaTime;
        // 가로등 밑 응시 판정은 연산량이 크므로 주기를 줄인다.
        if (WeirdStareCheckElapsed < 0.1f)
        {
            return;
        }
        WeirdStareCheckElapsed = 0.0f;

        FHitResult Hits;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Hotel_Walker);
        Params.AddIgnoredActor(AutoDoor);
        Params.AddIgnoredComponent(CapsuleComponent);
        FVector Start = GetActorLocation();
        FVector End = Hotel_Walker->GetActorLocation();
        Start.Z += 67; //시선 위치에서 
        End.Z += 67;    // 시선위치

        bool bIsLookingAtGuest = false;
        bool isHit = GetWorld()->LineTraceSingleByChannel(Hits, Start, End, ECC_Visibility, Params);
        if (!isHit)//캐릭터 사이에 아무것도 없으면
        {
            FVector TempVec1 = (Start - End);
            FVector TempVec2 = Hotel_Walker->FollowCamera->GetForwardVector();

            FVector TempVec3 = TempVec1;
            TempVec3.Z = 0;
            FVector TempVec4 = TempVec2;
            TempVec4.Z = 0;

            float Dot = FVector::DotProduct(TempVec3.GetSafeNormal(), TempVec4.GetSafeNormal()); //캐릭터의 수평 각도를 계산
            if (Dot > 0.7f) // 수평 방향으로 정면이다
            {
                float DeltaZ = (FaceCollision->GetComponentLocation().Z +10);
                float HorizontalDist = FVector(GetActorLocation() - Hotel_Walker->GetActorLocation()).Size2D();
                float VerticalAngle = FMath::RadiansToDegrees(FMath::Atan2(DeltaZ, HorizontalDist));    //
                float CameraPitch = Hotel_Walker->FollowCamera->GetComponentRotation().Pitch;

                float UpperLimit = 0.32f * VerticalAngle + 27.4f;
                float LowerLimit = -1.12f * VerticalAngle - 27.4f;

                bIsLookingAtGuest = (CameraPitch >= LowerLimit && CameraPitch <= UpperLimit);
            }
        }
        if (bIsLookingAtGuest != bWasLookingAtGuest)
        {
            if (bIsLookingAtGuest)
            {
                Hotel_Manager->OnEventTriggerAction("LookAtGuest");
            }
            else
            {
                Hotel_Manager->OnEventTriggerAction("NoLookAtGuest");
            }

            bWasLookingAtGuest = bIsLookingAtGuest;
        }
    }
}
// Called to bind functionality to input
void AHotel_Guest::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

FString AHotel_Guest::GetInteractMessage()
{
    return InteractMassage;
}
FName  AHotel_Guest::GetQuickActionName()
{
    return QuickActionName;
}

void AHotel_Guest::ExecuteActionByName(FName ActionName)
{

    if (TFunction<void()>* Func = availableAction.Find(ActionName))
    {
        (*Func)();
    }
}

void AHotel_Guest::SetHighLightInteractive(bool OnOff)
{
    if (!MeshComponents.IsEmpty())
    {
        if (OnOff)
        {
            for (USkeletalMeshComponent* MeshComp : MeshComponents)
            {
                if (MeshComp)
                {
                    MeshComp->SetCustomDepthStencilValue(1);  // 예: 1
                }
            }
        }
        else
        {
            for (USkeletalMeshComponent* MeshComp : MeshComponents)
            {
                if (MeshComp)
                {
                    MeshComp->SetCustomDepthStencilValue(0);  // 예: 1
                }
            }
        }
    }
}

void AHotel_Guest::ExecuteQuickAction()
{
    ExecuteActionByName(QuickActionName);
}

void AHotel_Guest::PlaySound(FName SoundKey)
{
    if (USoundBase** sb_Sound = SoundEffectMap.Find(SoundKey))
    {
        if (AudioComp->GetSound() != *sb_Sound)
        {
            AudioComp->SetSound(*sb_Sound);
            AudioComp->Play();
        }
    }
}

void AHotel_Guest::StopSound()
{
    AudioComp->SetSound(NULL);
    AudioComp->Stop();
}
void AHotel_Guest::OpenConversationUI()
{
    Hotel_Walker->ViewUIDialogue(this);
    /*
    if (IsValid(NowDialogueData))
    {
        Hotel_Walker->ViewUIDialogue(this);
    }
    else
    {
        NowDialogueData = Hotel_Manager->GetDialogueData(CheckNowDailogue(false));
        LastDialogueIndex = 0;
        if (NowDialogueData)
        {
            Hotel_Walker->ViewUIDialogue(this);
        }
    }*/
}

 UDialogueDataAsset* AHotel_Guest::GetDailogueData(EDialogueState DialogueState, bool isCall)
{
     FName DialogueName;
    switch (DialogueState)
    {
    case EDialogueState::DS_Guest_CheckIn:
    {
        if (IsReservationGuest)
        {
            DialogueName = "Guest_Check_In";
        }
        else
        {
            DialogueName = "Guest_Walk_In";
        }
    }
    break;
    case EDialogueState::DS_Guest_ComplainRoomCondition:
    {
        if (isCall)
        {
            DialogueName = "Guest_Phone_Complain_RoomDirty";
        }
        else
        {
            DialogueName = "Guest_Direct_Complain_RoomDirty";
        }
    }
    break;
    case EDialogueState::DS_Guest_Hanging:
        DialogueName = "Guest_Hanging";
        break;
    case EDialogueState::DS_Guest_HearingKnock:
        if (IsCalledWalker)
        {
            DialogueName = "Guest_HearingKnock_Waiting";
        }
        else
        {
            DialogueName = "Guest_HearingKnock";
        }
        break;
    case EDialogueState::DS_Guest_TresPass:
        {
        DialogueName = "Guest_TresPass";
        }
        break;
    case EDialogueState::DS_Guest_Default:
    {
        DialogueName = "Guest_Default";
    }
    break;
    
    case EDialogueState::DS_Guest_Apologize:
    {
        DialogueName = "Guest_Apologize_Accept";
    }
    break;

    case EDialogueState::DS_Guest_Complete_RoomClean:
    {
        DialogueName = "Guest_Complete_RoomClean";
    }
    break;
    

    default:
        DialogueName = "";
        break;
    }
    return Hotel_Manager->GetDialogueData(DialogueName);
}
void AHotel_Guest::SetComplainTimer()
{
    GetWorld()->GetTimerManager().SetTimer(ComplainTimer, this, &AHotel_Guest::ComplainToManager, 30, false); //30초동안 응답 없다면 연결끊김
}

void AHotel_Guest::ComplainToManager()
{
    Hotel_Manager->MinusHRScore(nPatience, TEXT("컴플레인 : 장시간 응답 대기 "));
}

void AHotel_Guest::AddDialogueDataState(FString title, EDialogueState state, int Index)
{
    if (arrGuestDialogueData.Num() > 2)
    {
        for (int i = 0; i < arrGuestDialogueData.Num(); i++)
        {
            if (arrGuestDialogueData[i].DialogueState == state)
            {
                arrGuestDialogueData.Swap(i, arrGuestDialogueData.Num() - 1);
                return; //같은 대화가 이미 목록에 있다면 우선순위 제일 뒤로 하고 리턴
            }
        }
    }

    FGuestDialogueData tempData;
    tempData.DialogueState = state;
    tempData.DialogueIndex = Index;
    tempData.DialogueTitle = title;
    arrGuestDialogueData.Add(tempData);
}
void AHotel_Guest::EraseDialgueDataState(EDialogueState state)
{
    for (int i = 0 ; i < arrGuestDialogueData.Num() ; i++)
    {
        if (arrGuestDialogueData[i].DialogueState == state)
        {
            arrGuestDialogueData.RemoveAt(i);
            return;
        }
    }
}

void AHotel_Guest::SetGuestDialogueData(UDialogueDataAsset* dialogueData, int DataIndex)
{
    NowDialogueData = dialogueData;
    LastGuestDialogueDataIndex = DataIndex;
}

void AHotel_Guest::SetGuestDialogueDataLast(bool isCalled)
{
    NowDialogueData = GetDailogueData(arrGuestDialogueData.Last().DialogueState, isCalled);
    LastGuestDialogueDataIndex = arrGuestDialogueData.Num() - 1;
}

void AHotel_Guest::SetDialoguePause(int GuestDialogueDataIndex, int Index)
{
    if (arrGuestDialogueData.Num() > GuestDialogueDataIndex)
    {
        if (NowDialogueData && Index > -1 && NowDialogueData->DialogueLines.Num() > Index)
        {
            arrGuestDialogueData[GuestDialogueDataIndex].DialogueIndex = Index;
        }
    }
}

void AHotel_Guest::EndGuestDialgue(int GuestDialogueDataIndex)
{
    if (arrGuestDialogueData.IsValidIndex(GuestDialogueDataIndex) && GuestDialogueDataIndex != 0)//기본 대화는 삭제 불가능
    {
        arrGuestDialogueData.RemoveAt(GuestDialogueDataIndex);
    }
    NowDialogueData = NULL;
}

void AHotel_Guest::SetHangingState(bool state)
{
    if (state)
    {
        IsHanging = true;
        if (IsCheckTrigger)Hotel_Manager->OnEventTriggerAction(FName(GuestName + "_HangingNeck"));
        if (AIController)AIController->StopMovement();
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->IsHanging = true;
        }
        if (IsValid(FaceAnimInstance))
        {
            if (IsWierdHanging)
            {
                IsReadyToNeckShaking = true;
            }
            else
            {
                FaceAnimInstance->IsHanging = true;
            }
        }

        if (GetCharacterMovement()) //중력 끄기
        {
            GetCharacterMovement()->GravityScale = 0.0f;
            GetCharacterMovement()->Velocity.Z = 0.0f;
        }
        availableAction.Empty();
        availableAction.Add(TEXT("수습"), [this]() {CorpseRetrieval(); });
        QuickActionName = TEXT("수습");
        UpdateInteractMessage();
    }
    else
    {
        IsHanging = false;
        BedyAnimInstance->IsHanging = false;
        availableAction.Empty();
        availableAction.Add(TEXT("대화"), [this]() {OpenConversationUI(); });
        QuickActionName = TEXT("대화");
        UpdateInteractMessage();
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->GravityScale = 1.0f;
            GetCharacterMovement()->Velocity.Z = 1.0f;
        }
    }
}

void AHotel_Guest::SetLookingCameraStatue(int state, AHotel_CCTV_Camera * camera)
{
    if (state > 1)
    {
        camera->SetNoiseOnceCamera(0.2f);
    }

    switch (state)
    {
    case 1:
    {
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->nCameraLookState = 0;
        }
        if (IsValid(FaceAnimInstance))
        {
            FaceAnimInstance->IsLaughing = false;
        }
        if (GetCharacterMovement())
        {
            GetCharacterMovement()->GravityScale = 1.0f;
            GetCharacterMovement()->Velocity.Z = 1.0f;
        }
    }
    break;
    case 2:
    {
        if (AIController)AIController->StopMovement();
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->nCameraLookState = 1;
        }
        SetLookActor(camera);
    }
    break;
    case 3:
    {
        if (AIController)AIController->StopMovement();
        if (AIController)AIController->StopMovement();
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->nCameraLookState = 1;
        }

        FVector relativeOffset(348, -52, -110);
        FVector worldPos = camera->GetActorTransform().TransformPosition(relativeOffset);
        TeleportTo(worldPos , FRotator(0.0f,0.0f,0.0f));
        SetLookActor(camera);
    }
    break;
    case 4:
    {
        if (AIController)AIController->StopMovement();
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->nCameraLookState = 2;
        }
        if (GetCharacterMovement()) //중력 끄기
        {
            GetCharacterMovement()->GravityScale = 0.0f;
            GetCharacterMovement()->Velocity.Z = 0.0f;
        }

        FVector relativeOffset(90, 0, -58);
        FVector worldPos = camera->GetActorTransform().TransformPosition(relativeOffset);
        TeleportTo(worldPos, FRotator(0.0f, 0.0f, 0.0f));
        SetLookActor(camera);
    }
    break;
    case 5:
    {
        if (AIController)AIController->StopMovement();
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->nCameraLookState = 2;
        }
        if (IsValid(FaceAnimInstance))
        {
            FaceAnimInstance->IsLaughing = true;
            PlaySound("Laugh");
        }
    }
    break;
    case 6:
    {
        if (AIController)AIController->StopMovement();
        if (IsValid(BedyAnimInstance))
        {
            BedyAnimInstance->nCameraLookState = 2;
        }

        TeleportTo(FVector(-1414, 2126, 94), FRotator(0.0f, 0.0f, 0.0f) , false, true);
        CatchingPlayer();
    }
    break;
    }

}



void AHotel_Guest::CorpseRetrieval()
{
    SetActorHiddenInGame(true);
    TeleportTo(FVector(-327, 839, 94),FRotator(0,0,0));
    DeactivateGuest();
    Hotel_Manager->WalkerCorpseRetrieval();
    CheckOutGuest();
}


void AHotel_Guest::DeactivateGuest()
{
    SetActorHiddenInGame(true);
    SetActorTickEnabled(false);
    SetActorEnableCollision(false);
    GetCharacterMovement()->GravityScale = 0.0f;//일단 중력을 끄고 액티브할때 켜중;
}

void AHotel_Guest::CheckOutGuest()
{
    if (!CheckInRoomNum.IsNone())
    {  
        Hotel_Manager->CheckOutGuestRoom(CheckInRoomNum);
        CheckInRoomNum = NAME_None;
    }
}

void AHotel_Guest::SetLookingPlayer(bool state) 
{ 
    if (IsHanging) return;
    IsLookingPlayer = state; 
}

void AHotel_Guest::GuestExit()
{
    AIController->StopPatienceTimer();
    AIController->StopAITimer();
    AIController->GoToOutside(); //밖으로 이동
}
void AHotel_Guest::SetNeckShakingState(bool state)
{
    IsReadyToNeckShaking = state;
}
void AHotel_Guest::ReleaseDoorLock()
{
    if (IsValid(WaitingDoor))
    {
        WaitingDoor->SetLockDoor(false);
        WaitingDoor->SetOpenDoor(true);
        AAI_Hotel_Guest_Default* Temp = Cast<AAI_Hotel_Guest_Default>(GetController());
        if (IsValid(Temp))
        {
            Temp->GoToRoom();
        }
    }
}

void AHotel_Guest::HearingKnockAction()
{
    if (IsHanging) return;
    Cast<AAI_Hotel_Guest_Default>(GetController())->HearingKnock();
}
void AHotel_Guest::ArrivedKnockingDoor(AHotel_Door* Door)
{
    WaitingDoor = Door;
    AddDialogueDataState(TEXT("노크소리 들음"), EDialogueState::DS_Guest_HearingKnock, 0);
    SetGuestDialogueDataLast(false);
    OpenConversationUI();
}

void AHotel_Guest::ReadyToMove()
{
    IsLookingPlayer = false;
    GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로 자동 회전 X
    bUseControllerRotationYaw = false; // 컨트롤러의 회전을 따름
}

void AHotel_Guest::ReadyToRotate()
{
    if (!bUseControllerRotationYaw)
    {
        GetCharacterMovement()->bOrientRotationToMovement = false; // 이동 방향으로 자동 회전 X
        bUseControllerRotationYaw = true; // 컨트롤러의 회전을 따름
    }
}

void AHotel_Guest::SetLookActor(AActor* targetActor)
{
    ReadyToRotate();
    FVector MyLocation = GetActorLocation();
    FVector TargetLocation = targetActor->GetActorLocation();
    FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(MyLocation, TargetLocation);
    SetActorRotation(FRotator(0, LookAtRot.Yaw, 0)); // 피치와 롤은 무시하고 Yaw만 사용 (수평 회전만)
}

void AHotel_Guest::CheckWalkerLookingFace(FVector GazeLocation, FVector PlayerGaze)
{
    if (!IsAlreadyNeckShaking)
    {
        FVector Face = GetActorLocation();
        Face.Z += 70.0f;//얼굴은 위쪽에 있다

        FVector ToFaceSphere = (Face - GazeLocation).GetSafeNormal();
        float Dot = FVector::DotProduct(GetActorForwardVector(), ToFaceSphere); //캐릭터가 정면에 있을시 체크를 위해

        if (Dot < 0)
        {
            FHitResult Hits;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(Hotel_Walker);
            Params.AddIgnoredComponent(CapsuleComponent);
            //얼굴을 위한 라인트레이스 함더
            bool isHit = GetWorld()->LineTraceSingleByChannel(Hits, GazeLocation, PlayerGaze, ECC_Visibility, Params);
            if (isHit)
            {
                if (Hits.GetComponent() == FaceCollision)
                {
                    if (IsWierdFaceLook)
                    {
                        Hotel_Manager->UpdateDefualtLevelMenual(8);
                        CatchingPlayer();
                    }
                    IsLookingFace = true;
                }
                else
                {
                    IsLookingFace = false;
                }
            }
        }
    }
}
void AHotel_Guest::CatchingPlayer()
{
    FVector PlayerLocation = Hotel_Walker->GetActorLocation();
    FVector CatcherLocation = GetActorLocation();

    FVector Direction = (CatcherLocation - PlayerLocation).GetSafeNormal();
    FVector TargetLocation = PlayerLocation + Direction * 60.0f;

    SetActorLocation(TargetLocation);
     
    Hotel_Walker->SetCatching(this);
    
    SetLookActor(Hotel_Walker);
    if (IsValid(BedyAnimInstance))
    {
        if (BedyAnimInstance->IsHanging) BedyAnimInstance->IsHanging = false;
        BedyAnimInstance->IsNeckShaking = true;
    }
    if (IsValid(FaceAnimInstance))
    {
        if (FaceAnimInstance->IsHanging) FaceAnimInstance->IsHanging = false;
        FaceAnimInstance->IsNeckShaking = true;
    }
    IsAlreadyNeckShaking = true;
    PlaySound("Scream");
}

void AHotel_Guest::CallingFailAction()
{
    if (IsHanging || CheckInRoomNum == NAME_None) return;
    AIController->CallingFail();
}