// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventorySlotWidget.h"
#include "UI/InventoryItemWidget.h"
#include "UI/InventoryItemDragDrop.h"
#include "UI/ReplicatedDragHolder.h"

#include "Item/InventorySlot.h"
#include "Item/ItemDataComponent.h"
#include "InventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Components/GridSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"

void UInventorySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	if (SizeBox_Root)
	{
		if (Button_SlotButton)
		{
			if (USizeBoxSlot* buttonAsSlot = Cast<USizeBoxSlot>(SizeBox_Root->AddChild(Button_SlotButton)))
			{
				buttonAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
				buttonAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				buttonAsSlot->SetPadding(0.f);
			}

			if (ItemWidget)
			{
				if(UItemDataComponent* itemData = GetItem())
					ItemWidget->SetItemInfo(itemData);
				ItemWidget->SetSize(SlotSize);

				if (UButtonSlot* itemWidgetAsSlot = Cast<UButtonSlot>(Button_SlotButton->AddChild(ItemWidget)))
				{
					itemWidgetAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
					itemWidgetAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
					itemWidgetAsSlot->SetPadding(0.f);
				}
			}
		}
	}
}

void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!Button_SlotButton) {
		return;
	}
	Button_SlotButton->SetIsEnabled(IsValid(GetItem()));

	Button_SlotButton->OnPressed.AddDynamic(ItemWidget, &UInventoryItemWidget::OnSlotButtonPressed);
	Button_SlotButton->OnHovered.AddDynamic(ItemWidget, &UInventoryItemWidget::OnSlotButtonHovered);
	Button_SlotButton->OnUnhovered.AddDynamic(ItemWidget, &UInventoryItemWidget::OnSlotButtonUnhovered);

	bIsFocusable = true;

}

void UInventorySlotWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	UItemDataComponent* itemData = GetItem();
	if (GetOwningPlayer()->IsInputKeyDown(DragItemMouseButton) || !IsValid(itemData)) {
		return;
	}
	FOnInputAction inputAction;
	inputAction.BindUFunction(itemData, FName("RotateItem"));
	ListenForInputAction(FName(RotateItemKey.GetDisplayName(false).ToString()), EInputEvent::IE_Pressed, false, inputAction);
	SetFocus();
}

void UInventorySlotWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	StopListeningForAllInputActions();
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	if (UInventoryItemDragDrop* itemDragDrop = Cast<UInventoryItemDragDrop>(InOperation))
	{
		if (IsValid(Inventory))
		{
			if(AReplicatedDragHolder* holder = Cast<AReplicatedDragHolder>(itemDragDrop->Payload))
			{
				if(UItemDataComponent* item = holder->CheckItem())
				{

					TArray<int> slots = Inventory->GetSlots(InventorySlotIndex, item->GetSize(), true);
					bool bCanFit= Inventory->SlotsAreEmpty(slots);
					itemDragDrop->OverSlot = this;
					itemDragDrop->SetSlotAvailable(bCanFit);
					return;
				}
			}
		}
		itemDragDrop->SetSlotAvailable(false);
		return;
	}
	
}

void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if(UInventoryItemDragDrop* itemOp = Cast< UInventoryItemDragDrop>(InOperation))
	{
		if(itemOp->OverSlot == this)
			itemOp->OverSlot = NULL;
	}
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	float fMouseDistance = FVector2D::Distance(InMouseEvent.GetScreenSpacePosition(), InMouseEvent.GetLastScreenSpacePosition());
	UE_LOG(LogTemp, Warning, TEXT("Drag Detected: %f"), fMouseDistance);
	if (fMouseDistance >= 1.f)
	{
		
		UInventoryItemDragDrop* itemOperation = Cast< UInventoryItemDragDrop>(UWidgetBlueprintLibrary::CreateDragDropOperation(UInventoryItemDragDrop::StaticClass()));
		UInventoryComponent* inventory = InventorySlot->GetInventory(); 
		UItemDataComponent* item = InventorySlot->GetItemInfo();
		
		if (IsValid(itemOperation) && IsValid(item) && IsValid(InventorySlot) && IsValid(inventory))
		{
			AReplicatedDragHolder* holder = inventory->GetItemHolder(item, InventorySlot->GetIndex());
			InventorySlot->GetInventory()->RemoveItem(InventorySlot->GetIndex());

			UInventoryItemWidget* itemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(ItemWidget->GetClass(), "Drag Item");
			itemWidget->SetItemInfo(item);
			itemWidget->SetSize(SlotSize);
			itemOperation->DefaultDragVisual = itemWidget;
			itemOperation->Tag = "Replicated Inventory";
			itemOperation->Payload = holder;
			itemOperation->UpdateOffset();

			GetOwningPlayer()->InputComponent->BindKey(RotateItemKey, EInputEvent::IE_Pressed, itemOperation, &UInventoryItemDragDrop::RecheckAvailability).bConsumeInput = false;
			GetOwningPlayer()->InputComponent->BindKey(RotateItemKey, EInputEvent::IE_Pressed, holder, &AReplicatedDragHolder::RotateHeldItem).bConsumeInput = false;
			GetOwningPlayer()->InputComponent->BindKey(RotateItemKey, EInputEvent::IE_Pressed, itemOperation, &UInventoryItemDragDrop::UpdateOffset);

			holder->BindReturnToDragCancel(itemOperation);

			OutOperation = itemOperation;
			
		}
	}
	
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UInventoryItemDragDrop* itemDragDrop = Cast<UInventoryItemDragDrop>(InOperation))
	{
		
		if (AReplicatedDragHolder* holder = Cast<AReplicatedDragHolder>(itemDragDrop->Payload))
		{
			if (UInventoryComponent* inventory = InventorySlot->GetInventory())
			{
				UItemDataComponent* item = holder->CheckItem();
				TArray<AInventorySlot*> slots = inventory->GetSlots(InventorySlot->GetIndex(), item->GetSize(), true);
				if(inventory->SlotsAreEmpty(slots))
				{
					inventory->AddItemToInventory(item->GetOwner(), InventorySlot->GetIndex());
					holder->RetrieveItem();
					holder->Destroy();
					return true;
				}
				return false;
			}
		}
	}

	return NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetPressedButtons().Contains(RotateItemKey))
	{
		InventorySlot->RotateItem();
	}
	else
	{
		FEventReply reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, DragItemMouseButton);
		if (reply.NativeReply.IsEventHandled())
			return reply.NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}



FReply UInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetPressedButtons().Contains(RotateItemKey))
	{
		// InventorySlot->RotateItem();
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UInventorySlotWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == RotateItemKey)
	{
		InventorySlot->RotateItem();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UInventorySlotWidget::SetInventorySlotIndex(AInventorySlot* newInventorySlot)
{
	if (!newInventorySlot) return;
	InventorySlot = newInventorySlot;
	InventorySlot->OnSlotUpdated.AddDynamic(this, &UInventorySlotWidget::UpdateItemInfo);

	UpdateItemInfo();
}

void UInventorySlotWidget::SetGridSlot(UGridSlot* gridPanelSlot, FVector2D slotSize)
{
	SlotSize = slotSize;

	if (!InventorySlot || !gridPanelSlot) return;
	GridPanelSlot = gridPanelSlot;
	if (SizeBox_Root)
	{
		FVector2D multiplier = FVector2D(1.f);
		if (InventorySlot->GetItemInfo())
		{
			FItemGridSize size = InventorySlot->GetItemInfo()->GetSize();
			multiplier.X = (size.Width);
			multiplier.Y = (size.Height);
		}
		SizeBox_Root->SetWidthOverride(slotSize.X * multiplier.X);
		SizeBox_Root->SetHeightOverride(slotSize.Y * multiplier.Y);
	}

	if (ItemWidget)
		ItemWidget->SetSize(slotSize);

	UpdateItemInfo();


}

void UInventorySlotWidget::UpdateItemInfo()
{
	if (!GridPanelSlot) return;
	FItemGridSize size = FItemGridSize(1);
	if (UItemDataComponent* item = InventorySlot->GetItemInfo()) {
		if (ItemWidget) {
			ItemWidget->SetItemInfo(item);
		}
		GridPanelSlot->SetLayer(1);
		size = item->GetSize();
		GridPanelSlot->SetColumnSpan(size.Width);
		GridPanelSlot->SetRowSpan(size.Height);
		if (Button_SlotButton) {
			Button_SlotButton->SetIsEnabled(true);
		}
	}
	else
	{
		if (ItemWidget)
			ItemWidget->SetItemInfo(NULL);
		GridPanelSlot->SetLayer(0);
		GridPanelSlot->SetColumnSpan(1);
		GridPanelSlot->SetRowSpan(1);
		if (Button_SlotButton)
		{
			Button_SlotButton->SetIsEnabled(false);
			Button_SlotButton->SetToolTipText(FText());
		}
	}

	if(SizeBox_Root)
	{
		SizeBox_Root->SetWidthOverride(SlotSize.X * size.Width);
		SizeBox_Root->SetHeightOverride(SlotSize.Y * size.Height);
	}
}

void UInventorySlotWidget::SetInteractInputs(FKey newRotate, FKey newDragMouseButton)
{
	RotateItemKey = newRotate;
	DragItemMouseButton = newDragMouseButton;
}

UItemDataComponent* UInventorySlotWidget::GetItem() const {
	if (!IsValid(Inventory)) {
		return nullptr;
	}
	return Inventory->GetItem(InventorySlotIndex);
}
