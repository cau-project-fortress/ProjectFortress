#include "FortressGameState.h"
#include "Net/UnrealNetwork.h"

AFortressGameState::AFortressGameState()
{
}

bool AFortressGameState::TryRemoveScrap(int32 Amount)
{
	if (!HasAuthority() || SharedScrapMetal < Amount)
	{
		return false;
	}
	SharedScrapMetal -= Amount;
	// Manually fire the rep notify on the server (listen server host won't receive OnRep via replication)
	OnRep_SharedScrapMetal();
	return true;
}

void AFortressGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AFortressGameState, SharedScrapMetal);
}

void AFortressGameState::OnRep_SharedScrapMetal()
{
	OnScrapMetalChanged.Broadcast(SharedScrapMetal);
}
