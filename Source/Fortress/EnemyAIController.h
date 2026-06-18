#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include"FortressExteriorPawn.h"

#include "EnemyAIController.generated.h"

UCLASS()
class FORTRESS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
    AEnemyAIController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target")
    USceneComponent* TargetComponent;

    UFUNCTION()
    void OnTargetDetected(AActor* Actor, FAIStimulus Stimulus);

    void UpdateBehavior();
    void ExecuteAttack();
    void ChooseNewTargetPart();

private:
    UPROPERTY(VisibleAnywhere)
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY()
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY()
    class ABaseEnemyCharacter* ControlledEnemy;

    UPROPERTY()
    AActor* CachedTarget;

    bool bIsTargetInSight = false;
    float LastFireTime = -1000.f;  // 충분히 과거로 초기화 (시작하자마자 쏠 수 있게)
    FTimerHandle BehaviorTimerHandle;
    FTimerHandle AttackTimerHandle;
};