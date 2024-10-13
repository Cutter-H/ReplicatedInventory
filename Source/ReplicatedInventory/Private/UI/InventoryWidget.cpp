// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryWidget.h"
#include "UI/InventorySlotWidget.h"
#include "UI/InventoryItemDragDrop.h"

#include "InventoryComponent.h"
#include "Item/ItemDataComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"

void UInventoryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox_Root)
	{

		if (GridPanel_Inventory)
		{
			if (USizeBoxSlot* gridAsSlot = Cast<USizeBoxSlot>(SizeBox_Root->AddChild(GridPanel_Inventory)))
			{
				gridAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				gridAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			}
		}
	}
}

void UInventoryWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryItemDragDrop* itemDD = Cast<UInventoryItemDragDrop>(InOperation))
	{
		itemDD->SetSlotAvailable(false);
	}
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

void UInventoryWidget::GenerateGrid()
{
	if (!IsValid(InventoryComponent) || !GridPanel_Inventory) return;
	UpdateGridSize();
	GridPanel_Inventory->ClearChildren();
	TArray<UItemDataComponent*>items= InventoryComponent->GetAllItems();
	for (int index = 0; index < items.Num(); index++) {

		TSubclassOf<UInventorySlotWidget> slotClass = UInventorySlotWidget::StaticClass();
		if (SlotWidgetClassOverride)
			slotClass = SlotWidgetClassOverride;

		UInventorySlotWidget* slot = WidgetTree->ConstructWidget<UInventorySlotWidget>(slotClass);

		slot->SetInventorySlotIndex(index);
		slot->SetInteractInputs(RotateInventoryItemKey, DragInventoryItemMouseButton);

		FItemGridSize size = FItemGridSize(1);
		if (UItemDataComponent * itemData = items[index]) {
			size = itemData->GetSize();
		}
		FInventory2DIndex gridIndex = InventoryComponent->IndexIntTo2D(index);

		if (UGridSlot* gridSlot = GridPanel_Inventory->AddChildToGrid(slot, gridIndex.Y, gridIndex.X)) {
			FVector2D slotSize = Size;
			if (SizeDistribution == EInventorySizeDistribution::SizeByGrid)
			{
				slotSize.X = slotSize.X / InventoryComponent->GetInventoryWidth();
				slotSize.Y = slotSize.Y / InventoryComponent->GetInventoryHeight();
			}
			slot->SetGridSlot(gridSlot, slotSize);
			gridSlot->SetPadding(0.f);
		}
	}
}

void UInventoryWidget::SetInventory(UInventoryComponent* newInventory)
{
	if (!IsValid(newInventory)) return;
	InventoryComponent = newInventory;
	GenerateGrid();
}

void UInventoryWidget::UpdateGridSize()
{
	if (SizeBox_Root)
	{
		if (SizeDistribution == EInventorySizeDistribution::SizeBySlot)
		{
			if (InventoryComponent)
			{
				SizeBox_Root->SetHeightOverride(Size.Y * InventoryComponent->GetInventoryHeight());
				SizeBox_Root->SetWidthOverride(Size.X * InventoryComponent->GetInventoryWidth());
			}
		}
		else
		{
			SizeBox_Root->SetHeightOverride(Size.Y);
			SizeBox_Root->SetWidthOverride(Size.X);
		}
	}
}
