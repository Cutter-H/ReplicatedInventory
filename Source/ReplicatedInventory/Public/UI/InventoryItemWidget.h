// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryItemWidget.generated.h"

class UItemDataComponent;

/**
 * 
 */
UCLASS()
class REPLICATEDINVENTORY_API UInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Item")
	void OnSlotButtonPressed();

	UFUNCTION()
	virtual void OnSlotButtonPressed_Internal();

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Item")
	void OnSlotButtonHovered();

	UFUNCTION()
	virtual void OnSlotButtonHovered_Internal();

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Item")
	void OnSlotButtonUnhovered();

	UFUNCTION()
	virtual void OnSlotButtonUnhovered_Internal();

	UFUNCTION(BlueprintNativeEvent, Category = "Inventory Item")
	void OnItemQuantityUpdate(int oldQuantity, int newQuantity);

	UFUNCTION()
	virtual void OnItemQuantityUpdate_Internal(int oldQuantity, int newQuantity);

	UFUNCTION()
	void SetItemInfo(UItemDataComponent* newItemData);

	UFUNCTION()
	void SetSize(FVector2D size);
	
	UFUNCTION()
	void SetTint(FLinearColor tint = FLinearColor::White);

	UFUNCTION()
	void SetFontSize(int newFontSize);

	//Refresh functions
	UFUNCTION()
	void UpdateWidget();

	UFUNCTION()
	void UpdateSize();

	UFUNCTION()
	void UpdateVisibility();

	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item")
	TObjectPtr<UItemDataComponent> ItemDataAsset;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Inventory Item")
	FVector2D BoxSize = FVector2D(100.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory Item")
	int FontSize = 10;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item", meta = (BindWidget))
	TObjectPtr<class USizeBox> SizeBox_Root;
	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item", meta = (BindWidget))
	TObjectPtr<class UOverlay> Overlay_Item;
	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item", meta = (BindWidget))
	TObjectPtr<class UImage> ItemImage;
	UPROPERTY(BlueprintReadWrite, Category = "Inventory Item", meta = (BindWidget))
	TObjectPtr<class UTextBlock> UIText;
};
