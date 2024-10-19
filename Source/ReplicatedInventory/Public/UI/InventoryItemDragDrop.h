// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryItemDragDrop.generated.h"

//class UUserWidget;

/**
 * 
 */
UCLASS()
class REPLICATEDINVENTORY_API UInventoryItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, Category = "Item")
	TObjectPtr<UUserWidget> OverSlot;

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetSlotAvailable(bool bAvailable);

	UFUNCTION(BlueprintCallable, Category = "Item")
	void UpdateOffset();

	UFUNCTION(BlueprintCallable, Category = "Item")
	void RecheckAvailability();
	
	//Payload should be the replicated holder.

	// DefaultDragVisual is the item's widget previously on the inventory board.
};
