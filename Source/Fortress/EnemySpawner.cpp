#include "EnemySpawner.h"
#include "BaseEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

#define PRINT_LOG(Caption) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, Caption);

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 서버에서만 스폰 로직 동작
    if (!HasAuthority()) return;

    if (bAutoStart)
    {
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnWave, SpawnInterval, true);
    }
}

AActor* AEnemySpawner::GetSpawnCenter()
{
    if (!IsValid(CachedTarget) && TargetClass)
    {
        CachedTarget = UGameplayStatics::GetActorOfClass(GetWorld(), TargetClass);
    }
    return CachedTarget;
}

void AEnemySpawner::SpawnWave()
{
    if (!HasAuthority()) return;

    AActor* Center = GetSpawnCenter();
    if (!Center)
    {
        PRINT_LOG(TEXT("스포너: 스폰 중심(탱크)을 찾지 못함!"));
        return;
    }

    const FVector CenterLocation = Center->GetActorLocation();

    // 이번 웨이브에 배치된 위치들 (겹침 체크용)
    TArray<FVector> PlacedPoints;

    // 클러스터 모드면 이번 웨이브 전체가 향할 "한 방향"을 먼저 뽑음
    const float ClusterAngleRad = FMath::FRandRange(0.f, 2.f * PI);

    for (const FEnemySpawnInfo& Info : SpawnList)
    {
        if (!Info.EnemyClass) continue;

        for (int32 i = 0; i < Info.CountPerWave; ++i)
        {
            FVector DesiredPoint;

            if (SpawnMode == EEnemySpawnMode::Scattered)
            {
                // 분산: 각 마리가 제각각 랜덤 각도 → 사방에 흩뿌림
                const float AngleRad = FMath::FRandRange(0.f, 2.f * PI);
                DesiredPoint = CenterLocation + FVector(
                    FMath::Cos(AngleRad) * Info.SpawnRadius,
                    FMath::Sin(AngleRad) * Info.SpawnRadius,
                    0.f
                );
            }
            else // Clustered
            {
                // 클러스터: 공통 방향의 한 지점을 중심으로, 작은 원 안에 랜덤 분포
                const FVector ClusterCenter = CenterLocation + FVector(
                    FMath::Cos(ClusterAngleRad) * Info.SpawnRadius,
                    FMath::Sin(ClusterAngleRad) * Info.SpawnRadius,
                    0.f
                );

                // 원 안에 균등 분포 (sqrt로 가장자리 쏠림 방지)
                const float R = ClusterSpread * FMath::Sqrt(FMath::FRand());
                const float Theta = FMath::FRandRange(0.f, 2.f * PI);
                DesiredPoint = ClusterCenter + FVector(
                    FMath::Cos(Theta) * R,
                    FMath::Sin(Theta) * R,
                    0.f
                );
            }

            TrySpawnOne(Info.EnemyClass, DesiredPoint, CenterLocation, PlacedPoints);
        }
    }
}

bool AEnemySpawner::TrySpawnOne(TSubclassOf<ABaseEnemyCharacter> EnemyClass, const FVector& DesiredPoint,
    const FVector& CenterLocation, TArray<FVector>& OutPlacedPoints)
{
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());

    FVector SpawnLocation = DesiredPoint;

    // NavMesh 위로 투영 (길 밖이면 적이 못 움직임)
    if (NavSys)
    {
        FNavLocation ProjectedLocation;
        if (NavSys->ProjectPointToNavigation(DesiredPoint, ProjectedLocation, FVector(1000.f, 1000.f, 5000.f)))
        {
            SpawnLocation = ProjectedLocation.Location;
        }
        else
        {
            PRINT_LOG(TEXT("스포너: NavMesh 투영 실패 → 건너뜀."));
            return false;
        }
    }

    // 겹침 체크: 이미 배치된 적들과 너무 가까우면 살짝 밀어서 재시도
    const float MinSepSq = MinSeparation * MinSeparation;
    const int32 MaxAttempts = 8;
    for (int32 Attempt = 0; Attempt < MaxAttempts; ++Attempt)
    {
        bool bTooClose = false;
        for (const FVector& Placed : OutPlacedPoints)
        {
            if (FVector::DistSquared2D(SpawnLocation, Placed) < MinSepSq)
            {
                bTooClose = true;
                break;
            }
        }

        if (!bTooClose) break;

        // 너무 가까우면 랜덤 방향으로 MinSeparation만큼 밀고 다시 NavMesh 투영
        const float PushAngle = FMath::FRandRange(0.f, 2.f * PI);
        FVector Pushed = SpawnLocation + FVector(FMath::Cos(PushAngle), FMath::Sin(PushAngle), 0.f) * MinSeparation;

        if (NavSys)
        {
            FNavLocation Reproj;
            if (NavSys->ProjectPointToNavigation(Pushed, Reproj, FVector(1000.f, 1000.f, 5000.f)))
            {
                SpawnLocation = Reproj.Location;
            }
        }
        else
        {
            SpawnLocation = Pushed;
        }
        // 마지막 시도까지 실패해도 그냥 마지막 위치로 배치 (약간 겹칠 수 있지만 드묾)
    }

    // 스폰 즉시 탱크를 바라보도록 회전
    FVector ToCenter = (CenterLocation - SpawnLocation).GetSafeNormal();
    ToCenter.Z = 0.f;
    const FRotator SpawnRotation = ToCenter.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ABaseEnemyCharacter* Spawned = GetWorld()->SpawnActor<ABaseEnemyCharacter>(
        EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (!Spawned)
    {
        PRINT_LOG(TEXT("스포너: 적 스폰 실패!"));
        return false;
    }

    OutPlacedPoints.Add(SpawnLocation);
    return true;
}