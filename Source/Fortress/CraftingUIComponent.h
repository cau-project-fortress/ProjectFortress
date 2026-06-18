#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingUIComponent.generated.h"

class AWorkbench;
class UWorkbenchWidget;

/**
 * Add this component to BP_FirstPersonCharacter.
 * When the server calls Client_ShowCraftingMenu, it creates/toggles WBP_CraftingMenu
 * using the CraftingWidgetClass set on the interacted workbench.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FORTRESS_API UCraftingUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCraftingUIComponent();

	/** Called by AWorkbench::ToggleCraftingUI on the server; executes on the owning client. */
	UFUNCTION(Client, Reliable)
	void Client_ShowCraftingMenu(AWorkbench* Workbench);


	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Crafting")
	void Server_RequestCraft(AWorkbench* Workbench, int32 ItemIndex);

private:
	UPROPERTY()
	TObjectPtr<UWorkbenchWidget> CraftingWidget;

	UPROPERTY()
	TObjectPtr<AWorkbench> CurrentWorkbench;
};
