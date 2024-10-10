// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/InventoryItemWidget.h"
#include "InventorySlotWidget.generated.h"

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

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void SetInventorySlot(class AInventorySlot* newInventorySlot);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void SetGridSlot(class UGridSlot* gridPanelSlot, FVector2D slotSize);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void UpdateItemInfo();

	UFUNCTION()
	void SetInteractInputs(FKey newRotate, FKey newDragMouseButton);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Slot")
	TObjectPtr<class AInventorySlot> InventorySlot;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory|Slot")
	TObjectPtr<class UGridSlot> GridPanelSlot;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory|Slot")
	FVector2D SlotSize;	

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Slot", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox_Root;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Slot", meta = (BindWidget))
	TObjectPtr<class UButton> Button_SlotButton;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory|Item", meta = (BindWidget))
	TObjectPtr<UInventoryItemWidget> ItemWidget;

	FKey RotateItemKey;

	FKey DragItemMouseButton;
	
};
