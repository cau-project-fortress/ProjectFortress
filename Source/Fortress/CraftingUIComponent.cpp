#include "CraftingUIComponent.h"
#include "Workbench.h"
#include "WorkbenchWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCraftingUIComponent::UCraftingUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

// 1. 링크 에러(LNK2001)를 해결하기 위한 RPC 함수 구현부 추가
bool UCraftingUIComponent::Server_RequestCraft_Validate(AWorkbench* Workbench, int ItemIndex)
{
	// 클라이언트의 요청이 유효한지 검증하는 로직 (일단 통과시키려면 true 반환)
	return true;
}

void UCraftingUIComponent::Server_RequestCraft_Implementation(AWorkbench* Workbench, int ItemIndex)
{
	// 실제 서버에서 아이템 제작(스크랩 차감, 인벤토리 추가 등)을 처리하는 로직
}

// -------------------------------------------------------------------------

void UCraftingUIComponent::Client_ShowCraftingMenu_Implementation(AWorkbench* Workbench)
{
	if (!Workbench) return;

	APawn* Pawn = Cast<APawn>(GetOwner());
	APlayerController* PC = Pawn ? Pawn->GetController<APlayerController>() : nullptr;
	if (!PC) return;

	// Toggle: close if this workbench's menu is already open
	if (CraftingWidget && CraftingWidget->IsInViewport() && CurrentWorkbench == Workbench)
	{
		// 2. Deprecated 경고(Warning C4996) 해결을 위해 RemoveFromParent()로 변경
		CraftingWidget->RemoveFromParent();

		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
		
		// disableMovement()이 아닌 ignoreInput으로 변경
		PC->ResetIgnoreMoveInput();
		PC->ResetIgnoreLookInput();

		Workbench->Server_CloseCraftingUI();
		CurrentWorkbench = nullptr;
		return;
	}

	// (Re)create widget if needed
	if (!CraftingWidget || CraftingWidget->GetOwningPlayer() != PC)
	{
		CraftingWidget = CreateWidget<UWorkbenchWidget>(PC, Workbench->CraftingWidgetClass);
		if (CraftingWidget)
		{
			CraftingWidget->OwningWorkbench = Workbench;
			CraftingWidget->SetupScrapBinding();
		}
	}

	if (!CraftingWidget) return;

	CurrentWorkbench = Workbench;
	CraftingWidget->OwningWorkbench = Workbench;
	CraftingWidget->SetupScrapBinding();
	CraftingWidget->AddToViewport();

	PC->bShowMouseCursor = true;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);

	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
}