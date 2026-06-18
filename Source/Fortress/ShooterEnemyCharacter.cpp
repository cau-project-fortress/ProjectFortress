#include "ShooterEnemyCharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"

void AShooterEnemyCharacter::Attack(AActor* Target) {
    if (!Target || !ProjectileClass) return;

    FVector ForwardVector = GetActorForwardVector();
    FVector Muzzle = GetActorLocation() + (ForwardVector * 100.f) + FVector(0.f, 0.f, 60.f);

    // 캐싱된 탄도 속도 → 회전으로 변환
    FRotator LaunchRot = CachedLaunchVelocity.Rotation();

    // ★ 좌우(Yaw) · 상하(Pitch)로 랜덤 흩뿌리기
    LaunchRot.Yaw += FMath::FRandRange(-SpreadAngle, SpreadAngle);
    LaunchRot.Pitch += FMath::FRandRange(-SpreadAngle, SpreadAngle);

    // 흩뿌린 방향으로 속도 벡터 재구성 (크기는 유지)
    FVector SpreadVelocity = LaunchRot.Vector() * CachedLaunchVelocity.Size();

    FActorSpawnParameters Params;
    Params.Instigator = this;
    Params.Owner = this;

    AActor* Proj = GetWorld()->SpawnActor<AActor>(ProjectileClass, Muzzle, LaunchRot, Params);

    if (Proj)
    {
        if (UProjectileMovementComponent* PMC = Proj->FindComponentByClass<UProjectileMovementComponent>())
        {
            PMC->Velocity = SpreadVelocity;
        }
    }
}