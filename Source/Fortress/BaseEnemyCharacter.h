#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "BaseEnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class EArmorType : uint8
{
    HeavyArmor  UMETA(DisplayName = "중장갑"),
    LightArmor  UMETA(DisplayName = "경장갑"),
    Suicide     UMETA(DisplayName = "자폭병")
};

UCLASS(Blueprintable)
class FORTRESS_API ABaseEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseEnemyCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, class AActor* DamageCauser) override;

    virtual void Attack(AActor* Target) {};

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
    TObjectPtr<UStaticMeshComponent> StaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 100.f;

    UPROPERTY(BlueprintReadOnly, Category = "Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSubclassOf<AActor> ProjectileClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    TSubclassOf<AActor> TargetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float FireRate = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AttackRange = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    bool bStopToAttack = true;

    UPROPERTY(EditAnywhere, Category = "Debug")
    FColor DebugArrowColor = FColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    FName PreferredTargetTag;

    UPROPERTY(BlueprintReadWrite, Category = "AI Combat")
    FVector CachedLaunchVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Combat")
    float SpreadAngle = 0.8f;  // 도(degree) 단위. 좌우·상하 ±3도

    // 이 적의 장갑 분류. 발사체가 히트 시 읽어서 차등 데미지 계산에 사용.
    // 각 적 BP 디테일 패널에서 드롭다운으로 지정 (중장갑 BP=HeavyArmor 등).
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI Combat")
    EArmorType ArmorType = EArmorType::LightArmor;

    void HandleDeath();

protected:
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Health")
    void OnDeath();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlayDeath();

private:

};