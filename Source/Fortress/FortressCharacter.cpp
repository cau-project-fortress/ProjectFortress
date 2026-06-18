#include "FortressCharacter.h"
#include "InteractionInterface.h"
#include "WorkbenchWidget.h"
#include "FortressGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

AFortressCharacter::AFortressCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFortressCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AFortressCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFortressCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 프로젝트 세팅의 "Interact"와 연결
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AFortressCharacter::Interact);
}

// 클래스 멤버임을 명시하기 위해 AFortressCharacter:: 가 반드시 필요합니다.
void AFortressCharacter::Interact()
{
	Server_Interact();
}

// 서버 실제 구현부
void AFortressCharacter::Server_Interact_Implementation()
{
	// 앉아 있는 경우: raycast 없이 현재 의자에 직접 인터랙트
	if (CurrentSeat && CurrentSeat->Implements<UInteractionInterface>())
	{
		IInteractionInterface::Execute_OnInteract(CurrentSeat, this);
		CurrentSeat = nullptr;
		return;
	}

	FVector Start = GetPawnViewLocation();
	FVector End = Start + (GetViewRotation().Vector() * InteractionDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->Implements<UInteractionInterface>())
		{
			CurrentSeat = HitActor;
			IInteractionInterface::Execute_OnInteract(HitActor, this);
		}
	}

	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 2.0f);
}

// 서버 검증부 (반드시 필요)
bool AFortressCharacter::Server_Interact_Validate()
{
	return true;
}

// ── Crafting UI ────────────────────────────────────────────────────────────────

int32 GCraftCosts_[] = { 25, 25, 20 }; // AP Shells, HE Shells, Repair Kit

void AFortressCharacter::Client_OpenCraftingUI_Implementation()
{
	if (!CraftingWidgetClass) return;

	if (!CraftingWidget)
	{
		CraftingWidget = CreateWidget<UWorkbenchWidget>(GetWorld(), CraftingWidgetClass);
	}

	if (!CraftingWidget || CraftingWidget->IsInViewport()) return;

	CraftingWidget->AddToViewport();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}

	GetCharacterMovement()->DisableMovement();
}

void AFortressCharacter::Client_CloseCraftingUI_Implementation()
{
	if (CraftingWidget && CraftingWidget->IsInViewport())
	{
		CraftingWidget->RemoveFromViewport();
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

bool AFortressCharacter::Server_RequestCraft_Validate(int32 ItemIndex)
{
	return ItemIndex >= 0 && ItemIndex < 3;
}

void AFortressCharacter::Server_RequestCraft_Implementation(int32 ItemIndex)
{
	if (AFortressGameState* GS = GetWorld()->GetGameState<AFortressGameState>())
	{
		GS->TryRemoveScrap(GCraftCosts_[ItemIndex]);
	}
}