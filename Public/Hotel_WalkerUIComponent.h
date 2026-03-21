#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Hotel_WalkerUIComponent.generated.h"

class UUserWidget;
class UInteractive_Box;
class UPhoneDialUI;
class UMenualUI;
class UDialogueUI;
class IInteractable_Object;
class AHotel_Phone;
class AHotel_Guest;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOOTS_ANKLE_HOTEL_API UHotel_WalkerUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHotel_WalkerUIComponent();

	void Initialize(class AHotel_Walker* InOwner, class APlayerController* PC);

	void GiveMouseControlTo(class UUserWidget* FocusWidget);
	void ReleaseMouseToGame();

	void UpdateInteractionPrompt(IInteractable_Object* Target);
	void HideInteractionPrompt();

	void ViewPhoneDial(class AHotel_Walker* Owner, AHotel_Phone* InteractedPhone);
	void ViewManual(class AHotel_Walker* Owner);
	void ViewDialogue(class AHotel_Walker* Owner, AHotel_Guest* DialogueGuest, AHotel_Phone* Phone = nullptr);

	void HideByLegacyName(FName UIName);
	bool IsLegacyNameVisible(FName UIName) const;

	void NotifyGameStartForManual();

	UInteractive_Box* GetInteractiveBox() const { return Interactive_Box; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walker|UI")
	TSubclassOf<UInteractive_Box> InteractiveBoxWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walker|UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walker|UI")
	TSubclassOf<UUserWidget> InteractiveWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walker|UI")
	TSubclassOf<UPhoneDialUI> PhoneUiWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walker|UI")
	TSubclassOf<UMenualUI> MenualUiWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walker|UI")
	TSubclassOf<UDialogueUI> DialogueUiWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Walker|UI")
	TObjectPtr<UInteractive_Box> Interactive_Box;

	UPROPERTY(BlueprintReadOnly, Category = "Walker|UI")
	TObjectPtr<UUserWidget> Crosshair;

	UPROPERTY(BlueprintReadOnly, Category = "Walker|UI")
	TObjectPtr<UUserWidget> InteractiveWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Walker|UI")
	TObjectPtr<UPhoneDialUI> PhoneUi;

	UPROPERTY(BlueprintReadOnly, Category = "Walker|UI")
	TObjectPtr<UMenualUI> MenualUi;

	UPROPERTY(BlueprintReadOnly, Category = "Walker|UI")
	TObjectPtr<UDialogueUI> DialogueUi;

protected:
	TWeakObjectPtr<class AHotel_Walker> WeakOwner;
	TWeakObjectPtr<class APlayerController> WeakPlayerController;

	void CreateAllWidgets();
};
