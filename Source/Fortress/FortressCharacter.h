#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FortressCharacter.generated.h"

class UWorkbenchWidget;

UCLASS()
class FORTRESS_API AFortressCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFortressCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 입력 설정을 위한 함수
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ── Crafting UI ───────────────────────────────────────────────────────────

	/** Widget class to use for the crafting menu. Set this on your BP_FirstPersonCharacter class defaults. */
	UPROPERTY(EditDefaultsOnly, Category = "Crafting")
	TSubclassOf<UWorkbenchWidget> CraftingWidgetClass;

	/** Opens the crafting UI on this client. Called by AWorkbench on the server. */
	UFUNCTION(Client, Reliable)
	void Client_OpenCraftingUI();

	/** Closes the crafting UI on this client. Called by AWorkbench on the server. */
	UFUNCTION(Client, Reliable)
	void Client_CloseCraftingUI();

	/** Sent from the widget to the server when the player clicks a craft button.
	 *  ItemIndex: 0 = AP Shells, 1 = HE Shells, 2 = Repair Kit */
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Crafting")
	void Server_RequestCraft(int32 ItemIndex);

protected:
	// 'E' 키 입력 시 호출될 함수
	void Interact();

	// 서버에서 실행될 실제 로직 (Server, Reliable, WithValidation 필수)
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact();

	// 상호작용 거리 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fortress|Interaction")
	float InteractionDistance = 300.0f;

	// 현재 앉아 있는 의자. 앉을 때 Chair BP에서 SetCurrentSeat로 설정, 일어날 때 nullptr로 클리어.
	UPROPERTY(BlueprintReadWrite, Category = "Fortress|Interaction")
	TObjectPtr<AActor> CurrentSeat = nullptr;

private:
	UPROPERTY()
	TObjectPtr<UWorkbenchWidget> CraftingWidget;
};