#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FortressPlayerController.generated.h"

class AFortressExteriorPawn;

/**
 * 서버 통신 및 로비 제어를 담당하는 컨트롤러입니다.
 */
UCLASS()
class FORTRESS_API AFortressPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** 서버에 게임 시작(맵 이동)을 요청합니다. */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Fortress|Network")
	void Server_StartGame(const FString& MapName);

	/**
	 * Call from BP_TankSeat's OnInteract event (pass the tank pawn reference).
	 * Caches the current character pawn, hides it, and possesses the tank.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Fortress|Vehicle")
	void Server_EnterVehicle(AFortressExteriorPawn* VehiclePawn);

	/**
	 * Call from AFortressExteriorPawn's exit input (E key while driving).
	 * Re-possesses the cached character and restores it.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Fortress|Vehicle")
	void Server_ExitVehicle();

	/** The character pawn stashed when we entered a vehicle. */
	UPROPERTY(BlueprintReadOnly, Category = "Fortress|Vehicle")
	TObjectPtr<APawn> CachedCharacterPawn;
};