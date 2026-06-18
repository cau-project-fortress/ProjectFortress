#include "FortressPlayerController.h"
#include "FortressExteriorPawn.h"

void AFortressPlayerController::Server_StartGame_Implementation(const FString& MapName)
{
	if (!HasAuthority()) return;
	UWorld* World = GetWorld();
	if (World)
	{
		FString TravelURL = MapName + TEXT("?listen");
		World->ServerTravel(TravelURL);
	}
}

void AFortressPlayerController::Server_EnterVehicle_Implementation(AFortressExteriorPawn* VehiclePawn)
{
	if (!HasAuthority() || !VehiclePawn) return;

	// Cache and hide the current character so it doesn't float in the air
	CachedCharacterPawn = GetPawn();
	if (CachedCharacterPawn)
	{
		CachedCharacterPawn->SetActorHiddenInGame(true);
		CachedCharacterPawn->SetActorEnableCollision(false);
	}

	Possess(VehiclePawn);
}

void AFortressPlayerController::Server_ExitVehicle_Implementation()
{
	if (!HasAuthority() || !CachedCharacterPawn) return;

	// Place the character next to the vehicle before re-possessing
	APawn* CurrentPawn = GetPawn();
	if (CurrentPawn)
	{
		FVector ExitOffset = CurrentPawn->GetActorRightVector() * 200.f;
		CachedCharacterPawn->SetActorLocation(CurrentPawn->GetActorLocation() + ExitOffset, false, nullptr, ETeleportType::TeleportPhysics);
	}

	Possess(CachedCharacterPawn);
	CachedCharacterPawn->SetActorHiddenInGame(false);
	CachedCharacterPawn->SetActorEnableCollision(true);
	CachedCharacterPawn = nullptr;
}