#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FortressGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScrapMetalChanged, int32, NewAmount);

UCLASS()
class FORTRESS_API AFortressGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AFortressGameState();

	/** Team-shared scrap metal pool. Starts at 100, replicated to all clients. */
	UPROPERTY(ReplicatedUsing = OnRep_SharedScrapMetal, BlueprintReadOnly, Category = "Fortress|Resources")
	int32 SharedScrapMetal = 100;

	/** Fired on clients when SharedScrapMetal changes, and on the server after TryRemoveScrap. */
	UPROPERTY(BlueprintAssignable, Category = "Fortress|Resources")
	FOnScrapMetalChanged OnScrapMetalChanged;

	/** Server-only: deduct Amount. Returns true on success, false if insufficient. */
	bool TryRemoveScrap(int32 Amount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_SharedScrapMetal();
};
