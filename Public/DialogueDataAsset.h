// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueDataAsset.generated.h"

/**
 * 
 */

enum EDialogueState
{
    DS_Guest_None, DS_Guest_Default, DS_Guest_TresPass, DS_Guest_CheckIn, DS_Guest_ComplainRoomCondition, DS_Guest_Hanging, DS_Guest_HearingKnock

    , DS_Manager_Warning, DS_Manager_Fire, DS_Manager_Request_Reject_CheckIn , DS_Manager_Allocate_Room_Guest
    , DS_Security
};

struct FGuestDialogueData
{
    FString DialogueTitle;
    EDialogueState DialogueState;
    int DialogueIndex;
    FTimerHandle DialogueTimer;
};

USTRUCT(BlueprintType)
struct FDialogueChoice
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Text;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int NextLineIndex;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName FunctionName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int FunctionParameter;
};

USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Line;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isPausable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool isDisconnectPhone = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueChoice> ChoicesButton;

};
UCLASS()
class BOOTS_ANKLE_HOTEL_API UDialogueDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueLine> DialogueLines;
};
