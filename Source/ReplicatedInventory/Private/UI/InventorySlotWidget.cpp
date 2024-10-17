// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventorySlotWidget.h"
#include "UI/InventoryItemWidget.h"
#include "UI/InventoryItemDragDrop.h"
#include "UI/ReplicatedDragHolder.h"

#include "Item/ItemDataComponent.h"
#include "InventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "Components/CanvasPanel.h"
#include "Components/GridSlot.h"
#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"

void UInventorySlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SizeBox_Root = WidgetTree->FindWidget<USizeBox>("SizeBox_Root");
	Button_SlotButton = WidgetTree->FindWidget<UButton>("Button_SlotButton");
	Widget_Item = WidgetTree->FindWidget<UInventoryItemWidget>("Widget_Item");

}
void UInventorySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	
		
	// Bindings of Delegates go here
	if (!Button_SlotButton) {
		return;
	}
	Button_SlotButton->SetIsEnabled(IsValid(GetItem()));

	Button_SlotButton->OnPressed.AddDynamic(Widget_Item, &UInventoryItemWidget::OnSlotButtonPressed);
	Button_SlotButton->OnHovered.AddDynamic(Widget_Item, &UInventoryItemWidget::OnSlotButtonHovered);
	Button_SlotButton->OnUnhovered.AddDynamic(Widget_Item, &UInventoryItemWidget::OnSlotButtonUnhovered);

	SetIsFocusable(true);

}

bool UInventorySlotWidget::Initialize() {
	//bool retVal = Super::Initialize();

	// Widget Creation
	
	if (!IsValid(SizeBox_Root)) {
	SizeBox_Root = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("SizeBox_Root"));
	}
	
	if (!IsValid(Button_SlotButton)) {
		Button_SlotButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("Button_SlotButton"));
	}
	
	if (!IsValid(Widget_Item)) {
		Widget_Item = WidgetTree->ConstructWidget<UInventoryItemWidget>(UInventoryItemWidget::StaticClass(), TEXT("Widget_Item"));
	}
	
	if (IsValid(SizeBox_Root)) {
		WidgetTree->RootWidget = SizeBox_Root;
		if (IsValid(Button_SlotButton)) {
			if (USizeBoxSlot* slot = Cast<USizeBoxSlot>(SizeBox_Root->AddChild(Button_SlotButton))) {
				slot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				slot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);

			}
			/* Item */
			if (IsValid(Widget_Item)) {
				if (UButtonSlot* itemWidgetAsSlot = Cast<UButtonSlot>(Button_SlotButton->AddChild(Widget_Item))) {
					itemWidgetAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
					itemWidgetAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
					itemWidgetAsSlot->SetPadding(0.f);
				}
				if (!IsDesignTime()) {
					if (IsValid(Inventory)) {
						if (UItemDataComponent* itemData = GetItem())
							Widget_Item->SetItemInfo(itemData);
						Widget_Item->SetSize(SlotSize);
					}
				}
			}
		}
	}

	return Super::Initialize();
}

void UInventorySlotWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();
}

void UInventorySlotWidget::InitializeNativeClassData() {
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
FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	if (InMouseEvent.GetPressedButtons().Contains(RotateItemKey)) {
		if (IsValid(GetItem())) {
			GetItem()->RotateItem();
		}
	}
	else {
		FEventReply reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, DragItemMouseButton);
		if (reply.NativeReply.IsEventHandled())
			return reply.NativeReply;
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
FReply UInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	if (InMouseEvent.GetPressedButtons().Contains(RotateItemKey)) {
		// InventorySlot->RotateItem();
	}
	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}
FReply UInventorySlotWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	if (InKeyEvent.GetKey() == RotateItemKey) {
		if (IsValid(GetItem())) {
			GetItem()->RotateItem();
		}
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
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
		
		if (IsValid(itemOperation) && IsValid(GetItem()) && IsValid(Inventory))
		{
			AReplicatedDragHolder* holder = Inventory->GetItemHolder(GetItem(), InventorySlotIndex);
			Inventory->RemoveItem(InventorySlotIndex);

			UInventoryItemWidget* itemWidget = WidgetTree->ConstructWidget<UInventoryItemWidget>(Widget_Item->GetClass(), "Drag Item");
			itemWidget->SetItemInfo(GetItem());
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
			if (IsValid(Inventory)) {
				UItemDataComponent* item = holder->CheckItem();
				TArray<int> slots = Inventory->GetSlots(InventorySlotIndex, item->GetSize(), true);
				if(Inventory->SlotsAreEmpty(slots)) {
					Inventory->AddItemToInventory(item->GetOwner(), InventorySlotIndex);
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

void UInventorySlotWidget::SetInventorySlotIndex(UInventoryComponent* newInventory, int newInventorySlot)
{
	if (Inventory = newInventory) {
		if (newInventorySlot < 0) {
			return;
		}
		InventorySlotIndex = newInventorySlot;
		Inventory->OnInventorySlotChange.AddDynamic(this, &UInventorySlotWidget::UpdateItemSlotInfo);
		UpdateItemSlotInfo(newInventorySlot, GetItem(), GetSlotState());
	}
}
void UInventorySlotWidget::SetGridSlot(UGridSlot* gridPanelSlot, FVector2D slotSize)
{
	SlotSize = slotSize;

	if (!gridPanelSlot) return;
	GridPanelSlot = gridPanelSlot;
	if (SizeBox_Root)
	{
		FVector2D multiplier = FVector2D(1.f);
		if (IsValid(GetItem())) {
			FItemGridSize size = GetItem()->GetSize();
			multiplier.X = (size.Width);
			multiplier.Y = (size.Height);
		}
		SizeBox_Root->SetWidthOverride(slotSize.X * multiplier.X);
		SizeBox_Root->SetHeightOverride(slotSize.Y * multiplier.Y);
	}

	if (Widget_Item)
		Widget_Item->SetSize(slotSize);

	UpdateItemSlotInfo(InventorySlotIndex, GetItem(), GetSlotState());


}
void UInventorySlotWidget::UpdateItemSlotInfo(int slotNum, UItemDataComponent* newItem, EInventorySlotState newSlotState)
{
	if (slotNum != InventorySlotIndex) {
		return;
	}
	if (!GridPanelSlot) {
		return;
	}
	FItemGridSize size = FItemGridSize(1);
	if (IsValid(newItem)) {
		if (Widget_Item) {
			Widget_Item->SetItemInfo(newItem);
		}
		GridPanelSlot->SetLayer(1);
		size = newItem->GetSize();
		GridPanelSlot->SetColumnSpan(size.Width);
		GridPanelSlot->SetRowSpan(size.Height);
		if (Button_SlotButton) {
			Button_SlotButton->SetIsEnabled(true);
		}
	}
	else
	{
		if (Widget_Item) {
			Widget_Item->SetItemInfo(NULL);
		}
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
EInventorySlotState UInventorySlotWidget::GetSlotState() const {
	if (IsValid(Inventory)) {
		if (GetItem()) {

		}
		if (Inventory->IsSlotTaken(InventorySlotIndex)) {
			return EInventorySlotState::Taken;
		} 
	}
	return EInventorySlotState::Empty;
}
UInventoryItemWidget* UInventorySlotWidget::CreateItemWidget() {
	return nullptr;
}
