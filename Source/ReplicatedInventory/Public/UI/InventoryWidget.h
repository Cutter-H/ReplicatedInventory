// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

UENUM(BlueprintType)
enum EInventorySizeDistribution
{
	SizeByGrid,
	SizeBySlot
};

/**
 * 
 */
UCLASS()
class REPLICATEDINVENTORY_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void GenerateGrid();
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventory(class UInventoryComponent* newInventory);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UpdateGridSize();

	UPROPERTY(BlueprintReadWrite, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox_Root;
		
	UPROPERTY(BlueprintReadWrite, Category = "Widgets", meta = (BindWidget))
	TObjectPtr<class UGridPanel> GridPanel_Inventory;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TEnumAsByte<EInventorySizeDistribution> SizeDistribution = EInventorySizeDistribution::SizeBySlot;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D Size = FVector2D(64.f);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<class UInventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, Category = "Inventory|Item")
	TSubclassOf< class UInventorySlotWidget> SlotWidgetClassOverride;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FKey RotateInventoryItemKey = FKey("LeftShift");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FKey DragInventoryItemMouseButton = FKey("RightMouseButton");
};
