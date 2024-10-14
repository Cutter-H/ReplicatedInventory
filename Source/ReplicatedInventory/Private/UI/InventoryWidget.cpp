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



bool UInventoryWidget::Initialize() {
	bool retVal = Super::Initialize();

	if (SizeBox_Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SizeBox_Root"))) {
		WidgetTree->RootWidget = SizeBox_Root;
		if (GridPanel_Inventory = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("GridPanel_Inventory"))) {
			if (USizeBoxSlot* slot = Cast<USizeBoxSlot>(SizeBox_Root->AddChild(GridPanel_Inventory))) {
				slot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				slot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
			}
			GenerateGrid();
		}
	}

	return retVal;
}

void UInventoryWidget::NativeConstruct()
{
	Super::NativePreConstruct();
	
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
	if (IsDesignTime()) {
		for (int index = 0; index < InventoryPreviewSize; index++) {
			TSubclassOf<UInventorySlotWidget> slotClass = UInventorySlotWidget::StaticClass();
			if (SlotWidgetClassOverride)
				slotClass = SlotWidgetClassOverride;
			UInventorySlotWidget* slot = WidgetTree->ConstructWidget<UInventorySlotWidget>(slotClass);
			slot->SetInventorySlotIndex(nullptr, index);
			int positionX = index % InventoryPreviewWidth;
			int positionY = index / InventoryPreviewWidth;
			if (UGridSlot* gridSlot = GridPanel_Inventory->AddChildToGrid(slot, positionY, positionX)) {
				FVector2D slotSize = Size;
				if (SizeDistribution == EInventorySizeDistribution::SizeByGrid) {
					slotSize.X = slotSize.X / InventoryComponent->GetInventoryWidth();
					slotSize.Y = slotSize.Y / InventoryComponent->GetInventoryHeight();
				}
				slot->SetGridSlot(gridSlot, slotSize);
				gridSlot->SetPadding(2.f);
			}
		}		
		return;
	}
	if (!IsValid(InventoryComponent) || !GridPanel_Inventory) return;
	UpdateGridSize();
	GridPanel_Inventory->ClearChildren();
	int invSize = InventoryComponent->GetInventorySize();
	for (int index = 0; index < invSize; index++) {

		TSubclassOf<UInventorySlotWidget> slotClass = UInventorySlotWidget::StaticClass();
		if (SlotWidgetClassOverride)
			slotClass = SlotWidgetClassOverride;

		UInventorySlotWidget* slot = WidgetTree->ConstructWidget<UInventorySlotWidget>(slotClass);

		slot->SetInventorySlotIndex(InventoryComponent, index);
		slot->SetInteractInputs(RotateInventoryItemKey, DragInventoryItemMouseButton);

		FItemGridSize size = FItemGridSize(1);
		if (UItemDataComponent * itemData = InventoryComponent->GetItem(index)) {
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
