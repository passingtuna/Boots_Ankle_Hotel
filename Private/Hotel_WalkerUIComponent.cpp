#include "Hotel_WalkerUIComponent.h"
#include "Hotel_Walker.h"
#include "Interactive_Box.h"
#include "PhoneDialUI.h"
#include "MenualUI.h"
#include "DialogueUI.h"
#include "Interactable_Object.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"

namespace HotelWalkerUI_Internal
{
static APlayerController* ResolvePlayerController(const TWeakObjectPtr<AHotel_Walker>& WeakOwner,
	const TWeakObjectPtr<APlayerController>& WeakPC)
{
	if (AHotel_Walker* Owner = WeakOwner.Get())
	{
		if (APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
		{
			return PC;
		}
	}
	return WeakPC.Get();
}
} // namespace HotelWalkerUI_Internal

UHotel_WalkerUIComponent::UHotel_WalkerUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHotel_WalkerUIComponent::Initialize(AHotel_Walker* InOwner, APlayerController* PC)
{
	WeakOwner = InOwner;
	WeakPlayerController = PC;
	if (!PC)
	{
		return;
	}
	CreateAllWidgets();
}

void UHotel_WalkerUIComponent::CreateAllWidgets()
{
	APlayerController* PC = WeakPlayerController.Get();
	AHotel_Walker* Owner = WeakOwner.Get();
	if (!PC || !Owner)
	{
		return;
	}

	if (InteractiveBoxWidgetClass)
	{
		Interactive_Box = CreateWidget<UInteractive_Box>(PC, InteractiveBoxWidgetClass);
		if (Interactive_Box)
		{
			Interactive_Box->AddToViewport();
			Interactive_Box->SetVisibility(ESlateVisibility::Hidden);
			Interactive_Box->SetHotelWalker(Owner);
		}
	}

	if (CrosshairWidgetClass)
	{
		Crosshair = CreateWidget<UUserWidget>(PC, CrosshairWidgetClass);
		if (Crosshair)
		{
			Crosshair->AddToViewport();
			Crosshair->SetVisibility(ESlateVisibility::Visible);
		}
	}

	if (InteractiveWidgetClass)
	{
		InteractiveWidget = CreateWidget<UUserWidget>(PC, InteractiveWidgetClass);
		if (InteractiveWidget)
		{
			InteractiveWidget->AddToViewport();
			InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (PhoneUiWidgetClass)
	{
		PhoneUi = CreateWidget<UPhoneDialUI>(PC, PhoneUiWidgetClass);
		if (PhoneUi)
		{
			PhoneUi->AddToViewport();
			PhoneUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (MenualUiWidgetClass)
	{
		MenualUi = CreateWidget<UMenualUI>(PC, MenualUiWidgetClass);
		if (MenualUi)
		{
			MenualUi->AddToViewport(10);
			MenualUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (DialogueUiWidgetClass)
	{
		DialogueUi = CreateWidget<UDialogueUI>(PC, DialogueUiWidgetClass);
		if (DialogueUi)
		{
			DialogueUi->AddToViewport();
			DialogueUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UHotel_WalkerUIComponent::GiveMouseControlTo(UUserWidget* FocusWidget)
{
	APlayerController* PC = HotelWalkerUI_Internal::ResolvePlayerController(WeakOwner, WeakPlayerController);
	if (!PC)
	{
		return;
	}

	PC->bShowMouseCursor = true;
	FInputModeUIOnly InputModeData;
	if (FocusWidget)
	{
		InputModeData.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputModeData);
	if (Crosshair)
	{
		Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHotel_WalkerUIComponent::ReleaseMouseToGame()
{
	APlayerController* PC = HotelWalkerUI_Internal::ResolvePlayerController(WeakOwner, WeakPlayerController);
	if (PC)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->bShowMouseCursor = false;
	}
	if (Crosshair)
	{
		Crosshair->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHotel_WalkerUIComponent::UpdateInteractionPrompt(IInteractable_Object* Target)
{
	if (!Target || !InteractiveWidget)
	{
		return;
	}
	if (UTextBlock* Text = Cast<UTextBlock>(InteractiveWidget->GetWidgetFromName(TEXT("InteractText"))))
	{
		Text->SetText(FText::FromString(Target->GetInteractMessage()));
	}
	InteractiveWidget->SetVisibility(ESlateVisibility::Visible);
}

void UHotel_WalkerUIComponent::HideInteractionPrompt()
{
	if (InteractiveWidget)
	{
		InteractiveWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UHotel_WalkerUIComponent::ViewPhoneDial(AHotel_Walker* Owner, AHotel_Phone* InteractedPhone)
{
	if (Owner)
	{
		Owner->TurnOffHighLight();
	}
	if (PhoneUi)
	{
		GiveMouseControlTo(PhoneUi);
		PhoneUi->SetVisibility(ESlateVisibility::Visible);
		PhoneUi->SetPhoneObject(InteractedPhone);
	}
}

void UHotel_WalkerUIComponent::ViewManual(AHotel_Walker* Owner)
{
	if (Owner)
	{
		Owner->TurnOffHighLight();
	}
	if (MenualUi)
	{
		GiveMouseControlTo(MenualUi);
		MenualUi->InitMenual(Owner);
		MenualUi->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHotel_WalkerUIComponent::ViewDialogue(AHotel_Walker* Owner, AHotel_Guest* DialogueGuest, AHotel_Phone* Phone)
{
	if (Owner)
	{
		Owner->TurnOffHighLight();
	}
	HideByLegacyName(FName(TEXT("InteractMessage")));
	if (DialogueUi)
	{
		GiveMouseControlTo(DialogueUi);
		DialogueUi->SetDialogueGuest(DialogueGuest, Phone);
		DialogueUi->SetHotelWalker(Owner);
		DialogueUi->SetVisibility(ESlateVisibility::Visible);
	}
}

void UHotel_WalkerUIComponent::HideByLegacyName(FName UIName)
{
	if (UIName == FName(TEXT("InteractBox")))
	{
		if (Interactive_Box)
		{
			Interactive_Box->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else if (UIName == FName(TEXT("InteractMessage")))
	{
		HideInteractionPrompt();
	}
	else if (UIName == FName(TEXT("Menual")))
	{
		if (MenualUi)
		{
			MenualUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else if (UIName == FName(TEXT("Dialogue")))
	{
		if (DialogueUi)
		{
			DialogueUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else if (UIName == FName(TEXT("Phone")))
	{
		if (PhoneUi)
		{
			PhoneUi->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

bool UHotel_WalkerUIComponent::IsLegacyNameVisible(FName UIName) const
{
	if (UIName == FName(TEXT("Phone")))
	{
		return PhoneUi && PhoneUi->GetVisibility() == ESlateVisibility::Visible;
	}
	if (UIName == FName(TEXT("InteractBox")))
	{
		return Interactive_Box && Interactive_Box->GetVisibility() == ESlateVisibility::Visible;
	}
	if (UIName == FName(TEXT("InteractMessage")))
	{
		return InteractiveWidget && InteractiveWidget->GetVisibility() == ESlateVisibility::Visible;
	}
	if (UIName == FName(TEXT("Menual")))
	{
		return MenualUi && MenualUi->GetVisibility() == ESlateVisibility::Visible;
	}
	if (UIName == FName(TEXT("Dialogue")))
	{
		return DialogueUi && DialogueUi->GetVisibility() == ESlateVisibility::Visible;
	}
	return false;
}

void UHotel_WalkerUIComponent::NotifyGameStartForManual()
{
	if (MenualUi)
	{
		MenualUi->SetGameStart();
		MenualUi->InitMenualText();
	}
}
