#include "Ladder.h"
#include "Components/BoxComponent.h"

ALadder::ALadder()
{
	PrimaryActorTick.bCanEverTick = false;

	HitVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("HitVolume"));
	SetRootComponent(HitVolume);

	HitVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitVolume->SetCollisionObjectType(ECC_WorldStatic);
	HitVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	BottomPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomPoint"));
	BottomPoint->SetupAttachment(RootComponent);

	TopPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopPoint"));
	TopPoint->SetupAttachment(RootComponent);
}

void ALadder::TeleportPlayer(APawn* Player)
{
	if (!Player) return;

	FVector PlayerLoc = Player->GetActorLocation();
	float DistSqToTop    = FVector::DistSquared(PlayerLoc, TopPoint->GetComponentLocation());
	float DistSqToBottom = FVector::DistSquared(PlayerLoc, BottomPoint->GetComponentLocation());

	FVector Dest = (DistSqToBottom <= DistSqToTop)
		? TopPoint->GetComponentLocation()
		: BottomPoint->GetComponentLocation();

	Player->TeleportTo(Dest, Player->GetActorRotation(), false, true);
}
