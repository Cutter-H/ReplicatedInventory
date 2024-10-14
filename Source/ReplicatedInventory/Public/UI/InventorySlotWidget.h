// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryDataTypes.h"
#include "InventorySlotWidget.generated.h"

class UInventoryComponent;
class UGridSlot;
class USizeBox;
class UButton;
class UInventoryItemWidget;
class UInventoryWidget;

/**
 * 
 */
UCLASS()
class REPLICATEDINVENTORY_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual bool Initialize() override;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent);

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void SetInventorySlotIndex(UInventoryComponent* newInventory, int newInventorySlot = -1);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void SetGridSlot(UGridSlot* gridPanelSlot, FVector2D slotSize);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void UpdateItemSlotInfo(int slotNum, UItemDataComponent* newItem, EInventorySlotState newSlotState);

	UFUNCTION()
	void SetInteractInputs(FKey newRotate, FKey newDragMouseButton);

	UFUNCTION()
	UItemDataComponent* GetItem() const;

	UFUNCTION()
	EInventorySlotState GetSlotState() const;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Slot")
	int InventorySlotIndex;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Slot")
	TObjectPtr<UInventoryComponent> Inventory;

	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Slot")
	TObjectPtr<UGridSlot> GridPanelSlot;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory|Slot")
	FVector2D SlotSize;	

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Slot")
	TObjectPtr<UButton> Button_SlotButton;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory|Item")
	TObjectPtr<UInventoryItemWidget> Widget_Item;

	TObjectPtr<UInventoryWidget> Widget_Inventory;

	FKey RotateItemKey;

	FKey DragItemMouseButton;

private:
	UInventoryItemWidget* CreateItemWidget();
	
};
