// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable_Object.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UInteractable_Object : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class BOOTS_ANKLE_HOTEL_API IInteractable_Object
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual TMap<FName, TFunction<void()>> GetAvailableAction() = 0;
    virtual FString GetInteractMessage() = 0;
    virtual FName GetQuickActionName() = 0;
	virtual void ExecuteActionByName(FName ActionName) = 0;
    virtual void SetHighLightInteractive(bool OnOff) = 0;
    virtual void ExecuteQuickAction() = 0;
    virtual void SetIsInteractive(bool on) = 0;
    virtual bool GetIsInteractive() = 0;
};
