#include "EnemyAIController.h"
#include "BaseEnemyCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"

#define PRINT_LOG(Caption) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Caption);

AEnemyAIController::AEnemyAIController()
{
    PRINT_LOG(TEXT("AI 컨트롤러 생성됨!"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // 시야 감각 설정
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.f; //감지 범위
        SightConfig->LoseSightRadius = 2500.f; // 감지 잃는 범위
        SightConfig->PeripheralVisionAngleDegrees = 180.f; // 감지 각도
        //감지 대상
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation()); //우선적으로 사용할 감각 설정
    }
}

void AEnemyAIController::BeginPlay()
{
    PRINT_LOG(TEXT("Enemy Begin Play..."));
    Super::BeginPlay();
    // delegate 바인딩
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetDetected);
    // 0.5초마다 이동/회전 판단
    GetWorldTimerManager().SetTimer(BehaviorTimerHandle, this, &AEnemyAIController::UpdateBehavior, 0.5f, true);
   
}

// 적 캐릭터 possess시 타겟 캐릭터를 찾아서 캐싱
void AEnemyAIController::OnPossess(APawn* InPawn)
{
    PRINT_LOG(TEXT("Possessing..."));
    Super::OnPossess(InPawn);
    ControlledEnemy = Cast<ABaseEnemyCharacter>(InPawn);

    if (ControlledEnemy)
    {
        PRINT_LOG(TEXT("Possessed!!!"));
        CachedTarget = UGameplayStatics::GetActorOfClass(GetWorld(), ControlledEnemy->TargetClass);
        bIsTargetInSight = true; // ★ 무조건 돌진: 빙의 즉시 항상 인지 상태로
    }
}

// 적 감지시 행동
void AEnemyAIController::OnTargetDetected(AActor* Actor, FAIStimulus Stimulus)
{
    // ★ 무조건 돌진 모드라 시야 감지로 정지시키지 않음 (Perception은 사실상 미사용)
}

void AEnemyAIController::UpdateBehavior()
{
    if (!ControlledEnemy) {
        PRINT_LOG(TEXT("Character 연결 실패!"));
        return;
    }
    if (!CachedTarget) {
        PRINT_LOG(TEXT("타겟(탱크)을 찾지 못함!"));
        return;
    }

    AFortressExteriorPawn* Fortress = Cast<AFortressExteriorPawn>(CachedTarget);
    if (Fortress && TargetComponent != Fortress->GetRootComponent())
    {
        FName AimedTag = ControlledEnemy->PreferredTargetTag;

        // 쏘고 있던 부위의 체력이 0?
        if (!Fortress->IsPartAlive(AimedTag))
        {
            ChooseNewTargetPart(); // 타겟 다시 골라라(몸통으로 변경됨)
        }
    }

    FVector AimLocation = TargetComponent ? TargetComponent->GetComponentLocation() : CachedTarget->GetActorLocation();

    // 타겟과 거리 계산
    float Dist = FVector::Dist(ControlledEnemy->GetActorLocation(), CachedTarget->GetActorLocation());

    // 1. 사거리보다 멀 때 -> 쫓아가기
    if (Dist > ControlledEnemy->AttackRange)
    {
        ControlledEnemy->DebugArrowColor = FColor::Yellow;
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);

        // ★ Z 보정 제거: MoveToActor가 타겟 위치를 알아서 NavMesh에 투영해줌
        AFortressExteriorPawn* FortressTarget = Cast<AFortressExteriorPawn>(CachedTarget);
        float AcceptRadius = FortressTarget ? FortressTarget->AcceptanceRadius : 100.f;

        EPathFollowingRequestResult::Type MoveResult = MoveToActor(CachedTarget, AcceptRadius);

        if (MoveResult == EPathFollowingRequestResult::Failed)
        {
            PRINT_LOG(TEXT("경로 탐색 실패! 갈 수 없는 곳임."));
        }
    }

    // 2. 사거리 이내로 들어왔을 때 -> 멈춰서 공격 시작
    else
    {
        StopMovement();
        ControlledEnemy->DebugArrowColor = FColor::Red;

        // --- 조준 로직 (그대로) ---
        FVector StartLocation = ControlledEnemy->GetActorLocation();
        FVector LaunchVelocity;
        float ProjectileSpeed = 20000.f;

        bool bHasAimSolution = UGameplayStatics::SuggestProjectileVelocity(
            this, LaunchVelocity, StartLocation, AimLocation,
            ProjectileSpeed, false, 0.f,
            GetWorld()->GetGravityZ(),
            ESuggestProjVelocityTraceOption::DoNotTrace
        );

        if (bHasAimSolution)
        {
            FRotator AimRotation = LaunchVelocity.Rotation();
            ControlledEnemy->SetActorRotation(FRotator(0.f, AimRotation.Yaw, 0.f));
            ControlledEnemy->CachedLaunchVelocity = LaunchVelocity;
        }
        else
        {
            FVector Dir = (AimLocation - ControlledEnemy->GetActorLocation()).GetSafeNormal();
            Dir.Z = 0.f;
            ControlledEnemy->SetActorRotation(Dir.Rotation());
        }

        // --- ★ 발사 판단: 쿨다운 지났으면 즉시 발사 ---
        float Now = GetWorld()->GetTimeSeconds();
        if (Now - LastFireTime >= ControlledEnemy->FireRate)
        {
            ExecuteAttack();
            LastFireTime = Now;
        }
    }
}

void AEnemyAIController::ExecuteAttack()
{
    if (!ControlledEnemy || !CachedTarget)
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
        return;
    }

    ControlledEnemy->Attack(CachedTarget);
}

void AEnemyAIController::ChooseNewTargetPart()
{
    if (!CachedTarget || !ControlledEnemy) return;

    AFortressExteriorPawn* Fortress = Cast<AFortressExteriorPawn>(CachedTarget);
    if (!Fortress) return;

    FName TargetTag = ControlledEnemy->PreferredTargetTag;

    if (TargetTag != NAME_None && Fortress->IsPartAlive(TargetTag))
    {
        TArray<UActorComponent*> FoundComponents = CachedTarget->GetComponentsByTag(USceneComponent::StaticClass(), TargetTag);

        if (FoundComponents.Num() > 0)
        {
            TargetComponent = Cast<USceneComponent>(FoundComponents[0]);
            PRINT_LOG(FString::Printf(TEXT("적 AI: 최우선 목표 [%s] 부위 조준 중..."), *TargetTag.ToString()));
            return;
        }
    }

    TargetComponent = CachedTarget->GetRootComponent();
    PRINT_LOG(TEXT("적 AI: 지정 부위 파괴됨 (또는 없음). 기본 몸통 조준 중..."));
}