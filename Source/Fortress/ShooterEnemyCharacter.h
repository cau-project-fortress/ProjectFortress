#pragma once

#include "CoreMinimal.h"
#include "BaseEnemyCharacter.h"
#include "ShooterEnemyCharacter.generated.h"

UCLASS()
class FORTRESS_API AShooterEnemyCharacter : public ABaseEnemyCharacter
{
	GENERATED_BODY()
	
	virtual void Attack(AActor* Target) override; // 공격 함수 구현

};
