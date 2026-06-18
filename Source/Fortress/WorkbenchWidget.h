#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WorkbenchWidget.generated.h"

class AWorkbench;

/**
 * C++ base for the crafting menu widget.
 * Create WBP_CraftingMenu in Blueprint extending this class.
 *
 * Buttons in the Blueprint widget should call:
 *   RequestCraftAPShells / RequestCraftHEShells / RequestCraftRepairKit
 *
 * Implement OnScrapAmountChanged in Blueprint to update the displayed counter.
 */
UCLASS()
class FORTRESS_API UWorkbenchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set automatically when the widget is created by AWorkbench::OnInteract_Implementation
	UPROPERTY(BlueprintReadOnly, Category = "Crafting")
	TObjectPtr<AWorkbench> OwningWorkbench;

	// ── Scrap costs (tweak in Blueprint defaults, no code change needed) ──────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Costs")
	int32 CostAPShells = 25;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Costs")
	int32 CostHEShells = 25;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crafting|Costs")
	int32 CostRepairKit = 20;

	// ── Called by Blueprint buttons ───────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestCraftAPShells();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestCraftHEShells();

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void RequestCraftRepairKit();

	// ── Implement in Blueprint to refresh the scrap counter text ─────────────
	UFUNCTION(BlueprintImplementableEvent, Category = "Crafting")
	void OnScrapAmountChanged(int32 NewAmount);

	/** Called by AWorkbench after OwningWorkbench is set. Binds the scrap delegate and pushes the current value. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void SetupScrapBinding();

protected:
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleScrapChanged(int32 NewAmount);
};
