#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Ladder.generated.h"

UCLASS()
class FORTRESS_API ALadder : public AActor
{
	GENERATED_BODY()

public:
	ALadder();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ladder")
	TObjectPtr<class UBoxComponent> HitVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ladder")
	TObjectPtr<USceneComponent> TopPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ladder")
	TObjectPtr<USceneComponent> BottomPoint;

public:
	UFUNCTION(BlueprintCallable, Category = "Ladder")
	void TeleportPlayer(APawn* Player);
};
