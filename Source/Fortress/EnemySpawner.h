#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class ABaseEnemyCharacter;

// 스폰 패턴 모드
UENUM(BlueprintType)
enum class EEnemySpawnMode : uint8
{
    Scattered   UMETA(DisplayName = "Scattered (전방위 분산)"),
    Clustered   UMETA(DisplayName = "Clustered (한 방향 뭉치기)")
};

// 적 종류별 스폰 설정 (에디터에서 항목 추가)
USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
    GENERATED_BODY()

    // 스폰할 적 클래스 (BP_RangedEnemy, BP_ExplodeEnemy 등)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TSubclassOf<ABaseEnemyCharacter> EnemyClass;

    // 탱크 중심으로부터의 스폰 반경 (cm 단위! 500m = 50000, 150m = 15000)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 50000.f;

    // 이 종류를 웨이브당 몇 마리 스폰할지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ClampMin = "1"))
    int32 CountPerWave = 3;
};

UCLASS()
class FORTRESS_API AEnemySpawner : public AActor
{
    GENERATED_BODY()

public:
    AEnemySpawner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TSubclassOf<AActor> TargetClass;

    // 적 종류별 스폰 설정 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TArray<FEnemySpawnInfo> SpawnList;

    // 스폰 패턴: 분산 / 클러스터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    EEnemySpawnMode SpawnMode = EEnemySpawnMode::Scattered;

    // [클러스터 모드] 뭉치는 원의 반경 (cm). 작으면 빽빽, 크면 느슨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (EditCondition = "SpawnMode == EEnemySpawnMode::Clustered"))
    float ClusterSpread = 2000.f;

    // 적들 사이 최소 간격 (cm). 메쉬 겹침 방지. 캡슐 지름보다 크게
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinSeparation = 150.f;

    // 스폰 주기 (초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnInterval = 5.f;

    // 시작 시 자동으로 스폰 시작
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bAutoStart = true;

private:
    void SpawnWave();
    AActor* GetSpawnCenter();

    // 한 마리 스폰 시도 (NavMesh 투영 + 겹침 체크 포함)
    bool TrySpawnOne(TSubclassOf<ABaseEnemyCharacter> EnemyClass, const FVector& DesiredPoint,
        const FVector& CenterLocation, TArray<FVector>& OutPlacedPoints);

    UPROPERTY()
    AActor* CachedTarget;

    FTimerHandle SpawnTimerHandle;
};