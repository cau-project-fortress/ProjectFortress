#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Workbench.generated.h"

class UWorkbenchWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorkbenchScrapChanged, int32, NewAmount);

UCLASS()
class FORTRESS_API AWorkbench : public AActor
{
	GENERATED_BODY()

public:
	AWorkbench();

	/** Shared scrap pool for this workbench. Replicated to all clients. */
	UPROPERTY(ReplicatedUsing = OnRep_SharedScrapMetal, BlueprintReadOnly, Category = "Crafting")
	int32 SharedScrapMetal = 100;

	/** Fires on both server and clients when scrap changes. */
	UPROPERTY(BlueprintAssignable, Category = "Crafting")
	FOnWorkbenchScrapChanged OnScrapMetalChanged;

	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void AddScrap(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Crafting", meta = (WorldContext = "WorldContextObject"))
	static void AddScrapToWorld(const UObject* WorldContextObject, int32 Amount);

	/** Server-only: deduct Amount. Returns true if successful. */
	bool TryRemoveScrap(int32 Amount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	/** Set to your WBP_CraftingMenu Blueprint widget on the BP_Workbench class defaults. */
	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	TSubclassOf<UWorkbenchWidget> CraftingWidgetClass;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Workbench")
	TObjectPtr<class UStaticMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<UWorkbenchWidget> CraftingWidget;

private:
	UFUNCTION()
	void OnRep_SharedScrapMetal();

public:
	/** Called from BPI_Interact Blueprint event to open or close the crafting UI. */
	UFUNCTION(BlueprintCallable, Category = "Crafting")
	void ToggleCraftingUI(APlayerController* PC);

	/** Called by widget craft buttons. ItemIndex: 0=AP Shells, 1=HE Shells, 2=Repair Kit. */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Crafting")
	void Server_RequestCraft(int32 ItemIndex);

	/** Called by CraftingUIComponent when the client closes the menu; clears net ownership. */
	UFUNCTION(Server, Reliable)
	void Server_CloseCraftingUI();

private:
	/** Tracks which remote PC currently has this workbench's menu open (server-only). */
	UPROPERTY()
	TObjectPtr<APlayerController> CurrentOpenPC;
};
