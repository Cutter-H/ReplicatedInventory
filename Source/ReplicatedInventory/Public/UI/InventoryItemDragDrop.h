// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryItemDragDrop.generated.h"

/**
 * 
 */
UCLASS()
class REPLICATEDINVENTORY_API UInventoryItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TObjectPtr<class UInventorySlotWidget> OverSlot;

	UFUNCTION()
	void SetSlotAvailable(bool bAvailable);

	UFUNCTION()
	void UpdateOffset();

	UFUNCTION()
	void RecheckAvailability();
	
	//Payload should be the replicated holder.

	// DefaultDragVisual is the item's widget previously on the inventory board.
};
