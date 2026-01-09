// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Hotel_Types.h"
#include "EdGraph/EdGraphNode.h"
#include "DialogueDataAsset.generated.h"

/**
 * 
 */

UCLASS()
class BOOTS_ANKLE_HOTEL_API UDialogueDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDialogueLine> DialogueLines;
};
