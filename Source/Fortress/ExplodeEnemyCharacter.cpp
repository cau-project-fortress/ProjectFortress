#include "ExplodeEnemyCharacter.h"
#include "Kismet/GameplayStatics.h"

#define PRINT_LOG(Caption) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Caption);

AExplodeEnemyCharacter::AExplodeEnemyCharacter()
{
	bStopToAttack = false; // ★ 자폭병은 멈추지 않고 계속 돌진
}

void AExplodeEnemyCharacter::Attack(AActor* Target)
{
	PRINT_LOG(TEXT("!!폭발!!"));
	UGameplayStatics::ApplyRadialDamage(this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr, TArray<AActor*>(), this, GetController(), true);
	HandleDeath();
}