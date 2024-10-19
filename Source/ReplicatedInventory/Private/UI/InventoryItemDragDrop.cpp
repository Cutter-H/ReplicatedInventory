// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryItemDragDrop.h"
#include "UI/InventoryItemWidget.h"
#include "UI/ReplicatedDragHolder.h"
#include "UI/InventorySlotWidget.h"

#include "InventoryComponent.h"
#include "Item/ItemDataComponent.h"


void UInventoryItemDragDrop::SetSlotAvailable(bool bAvailable)
{
	if (UInventoryItemWidget* itemWidget = Cast<UInventoryItemWidget>(DefaultDragVisual))
	{
		if (bAvailable)
			itemWidget->SetTint(FLinearColor::Green);
		else
			itemWidget->SetTint(FLinearColor::Red);
	}
}

void UInventoryItemDragDrop::UpdateOffset()
{
	Pivot = EDragPivot::TopLeft;
	if(UInventoryItemWidget* dragWidget = Cast<UInventoryItemWidget>(DefaultDragVisual))
	{
		if (UItemDataComponent* item = dragWidget->ItemDataAsset)
		{
			FItemGridSize itemSize = item->GetSize();
			Offset =
			FVector2D(
				FMath::Pow(0.5f, itemSize.Width) * -1.f,
				FMath::Pow(0.5f, itemSize.Height) * -1.f
			);
			dragWidget->UpdateWidget();
		}
	}
}

void UInventoryItemDragDrop::RecheckAvailability()
{
	if(!IsValid(OverSlot))
	{
		SetSlotAvailable(false);
		return;
	}/*
	if (UInventoryComponent* inventory = OverSlot->Inventory)
	{
		if(UInventoryItemWidget* dragWidget = Cast<UInventoryItemWidget>(DefaultDragVisual))
		{
			if (UItemDataComponent* item = dragWidget->ItemDataAsset)
			{

				TArray<int> slots = inventory->GetSlots(OverSlot->InventorySlotIndex, item->GetSize(), true);
				bool bCanFit = inventory->SlotsAreEmpty(slots);

				SetSlotAvailable(!bCanFit);
				return;
			}
		}
	}/**/
}
