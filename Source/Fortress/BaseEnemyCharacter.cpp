#include "BaseEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

ABaseEnemyCharacter::ABaseEnemyCharacter(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer
        .DoNotCreateDefaultSubobject(ACharacter::MeshComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

    StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    StaticMesh->SetupAttachment(GetCapsuleComponent());

    bReplicates = true;
    SetReplicateMovement(true);

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    GetCharacterMovement()->bEnablePhysicsInteraction = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ABaseEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

float ABaseEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (DamageCauser && DamageCauser->IsA<ABaseEnemyCharacter>())
    {
        return 0.f;
    }

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage <= 0.f || CurrentHealth <= 0.f) return 0.f;

    CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.f);
    if (CurrentHealth <= 0.f) HandleDeath();

    return ActualDamage;
}

void ABaseEnemyCharacter::HandleDeath()
{
    Multicast_PlayDeath();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    if (HasAuthority())
    {
        SetLifeSpan(0.5f);
    }
}

void ABaseEnemyCharacter::Multicast_PlayDeath_Implementation()
{
    OnDeath();
}

void ABaseEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING 
    FVector EyesLocation;
    FRotator EyesRotation;
    GetActorEyesViewPoint(EyesLocation, EyesRotation);

    FVector ForwardVector = EyesRotation.Vector();
    FVector EndLocation = EyesLocation + (ForwardVector * 150.f);

    DrawDebugDirectionalArrow(
        GetWorld(),
        EyesLocation,
        EndLocation,
        50.f,
        DebugArrowColor,
        false,
        -1.f,
        0,
        2.f
    );
#endif
}