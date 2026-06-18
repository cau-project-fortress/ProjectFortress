#include "Workbench.h"
#include "CraftingUIComponent.h"
#include "Kismet/GameplayStatics.h"
#include "WorkbenchWidget.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


AWorkbench::AWorkbench()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AWorkbench::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWorkbench, SharedScrapMetal);
}

bool AWorkbench::TryRemoveScrap(int32 Amount)
{
	if (!HasAuthority() || SharedScrapMetal < Amount) return false;
	SharedScrapMetal -= Amount;
	OnRep_SharedScrapMetal(); // fire on server (listen server won't get OnRep via replication)
	return true;
}

void AWorkbench::AddScrap(int32 Amount)
{
	// 서버 권한에서만 변경. 클라가 호출하면 무시됨.
	if (!HasAuthority() || Amount <= 0) return;
	SharedScrapMetal += Amount;
	OnRep_SharedScrapMetal(); // 리슨 서버 UI 갱신용으로 서버에서 직접 호출
}

void AWorkbench::AddScrapToWorld(const UObject* WorldContextObject, int32 Amount)
{
	UWorld* World = GEngine
		? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)
		: nullptr;
	if (!World) return;

	// 워크벤치가 월드에 하나라는 전제. 여러 개면 GetAllActorsOfClass로 바꿔야 함.
	if (AWorkbench* Workbench = Cast<AWorkbench>(
		UGameplayStatics::GetActorOfClass(World, AWorkbench::StaticClass())))
	{
		Workbench->AddScrap(Amount); // 내부에서 HasAuthority 가드
	}
}

void AWorkbench::OnRep_SharedScrapMetal()
{
	OnScrapMetalChanged.Broadcast(SharedScrapMetal);
}

void AWorkbench::ToggleCraftingUI(APlayerController* PC)
{
	if (!PC || !CraftingWidgetClass) return;

	// If we're on the server and this PC belongs to a remote client, dispatch via
	// Client RPC — widgets cannot be created server-side for remote players.
	if (HasAuthority() && !PC->IsLocalController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			if (UCraftingUIComponent* CraftingComp = Pawn->FindComponentByClass<UCraftingUIComponent>())
			{
				SetOwner(PC);  // grant net ownership so the client can call Server RPCs on this actor
				CraftingComp->Client_ShowCraftingMenu(this);
			}
		}
		return;
	}

	// Widgets can only be created on the owning client.
	if (!PC->IsLocalController()) return;

	// Close if already open
	if (CraftingWidget && CraftingWidget->IsInViewport())
	{
		CraftingWidget->RemoveFromViewport();
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());

		// set movement mode 아닌 resetIgnore으로 변경
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();

		SetOwner(nullptr);
		return;
	}

	// Create if needed
	if (!CraftingWidget || CraftingWidget->GetOwningPlayer() != PC)
	{
		CraftingWidget = CreateWidget<UWorkbenchWidget>(PC, CraftingWidgetClass);
		if (CraftingWidget)
		{
			CraftingWidget->OwningWorkbench = this;
			CraftingWidget->SetupScrapBinding();
		}
	}

	if (!CraftingWidget) return;

	SetOwner(PC);
	CraftingWidget->SetupScrapBinding();
	CraftingWidget->AddToViewport();
	PC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);

	// disableMovement()이 아닌 ignoreInput으로 변경
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
}

void AWorkbench::Server_CloseCraftingUI_Implementation()
{
	SetOwner(nullptr);
}

static const int32 GCraftCosts[] = { 25, 25, 20 }; // AP Shells, HE Shells, Repair Kit

bool AWorkbench::Server_RequestCraft_Validate(int32 ItemIndex)
{
	return ItemIndex >= 0 && ItemIndex < 3;
}

void AWorkbench::Server_RequestCraft_Implementation(int32 ItemIndex)
{
	if (TryRemoveScrap(GCraftCosts[ItemIndex]))
	{
		if (ItemIndex == 0 || ItemIndex == 1)
		{
			TArray<AActor*> FoundAmmoBoxes;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundAmmoBoxes);

			for (AActor* Actor : FoundAmmoBoxes)
			{
				if (Actor && Actor->GetName().StartsWith(TEXT("BP_AmmoBox")))
				{
					FProperty* BoxTypeProp = Actor->GetClass()->FindPropertyByName(TEXT("BoxType"));
					FProperty* AmmoCountProp = Actor->GetClass()->FindPropertyByName(TEXT("StoredAmmoCount"));

					if (BoxTypeProp && AmmoCountProp)
					{
						int32* BoxTypeValue = BoxTypeProp->ContainerPtrToValuePtr<int32>(Actor);
						int32* AmmoCountValue = AmmoCountProp->ContainerPtrToValuePtr<int32>(Actor);

						if (BoxTypeValue && AmmoCountValue)
						{
							if (ItemIndex == 0 && *BoxTypeValue == 1)
							{
								*AmmoCountValue += 1;
							}
							else if (ItemIndex == 1 && *BoxTypeValue == 2)
							{
								*AmmoCountValue += 1;
							}
						}
					}
				}
			}
		}
	}
}