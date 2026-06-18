#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyCharacter.h"
#include "ExplodeEnemyCharacter.generated.h"

UCLASS()
class FORTRESS_API AExplodeEnemyCharacter : public ABaseEnemyCharacter
{
	GENERATED_BODY()

public:
	AExplodeEnemyCharacter();

	virtual void Attack(AActor* Target) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ExplosionDamage = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float ExplosionRadius = 500.f;
};