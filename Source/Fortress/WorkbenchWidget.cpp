#include "WorkbenchWidget.h"
#include "Workbench.h"

void UWorkbenchWidget::SetupScrapBinding()
{
	if (!OwningWorkbench) return;
	OwningWorkbench->OnScrapMetalChanged.AddDynamic(this, &UWorkbenchWidget::HandleScrapChanged);
	// Push the current value immediately so the display is correct on open
	OnScrapAmountChanged(OwningWorkbench->SharedScrapMetal);
}

void UWorkbenchWidget::NativeDestruct()
{
	if (OwningWorkbench)
	{
		OwningWorkbench->OnScrapMetalChanged.RemoveDynamic(this, &UWorkbenchWidget::HandleScrapChanged);
	}

	Super::NativeDestruct();
}

void UWorkbenchWidget::HandleScrapChanged(int32 NewAmount)
{
	OnScrapAmountChanged(NewAmount);
}

void UWorkbenchWidget::RequestCraftAPShells()
{
	if (OwningWorkbench)
	{
		OwningWorkbench->Server_RequestCraft(0);
	}
}

void UWorkbenchWidget::RequestCraftHEShells()
{
	if (OwningWorkbench)
	{
		OwningWorkbench->Server_RequestCraft(1);
	}
}

void UWorkbenchWidget::RequestCraftRepairKit()
{
	if (OwningWorkbench)
	{
		OwningWorkbench->Server_RequestCraft(2);
	}
}
