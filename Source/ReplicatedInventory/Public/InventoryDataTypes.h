// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryDataTypes.generated.h"

class UInventoryItemData;

UENUM(BlueprintType)
enum EInventorySlotState {
	Empty,
	Taken,
	Used
};

UENUM(BlueprintType)
enum EItemModify {
	Override,
	Add,
	Subtract
};

USTRUCT(BlueprintType)
struct FItemGridSize {
	GENERATED_BODY()
public:
	FItemGridSize() {
		Width = 1;
		Height = 1;
	}
	FItemGridSize(int squareSize) {
		Width = squareSize;
		Height = squareSize;
	}
	FItemGridSize(int width, int height) {
		Width = width;
		Height = height;
	}

	FItemGridSize GetFlipped() const { return FItemGridSize(Height, Width); }
	bool IsSingle() const { return ((Width == 1) && (Height == 1)); }
	bool IsSquare() const {	return (Width == Height); }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Size")
	int Width = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Size")
	int Height = 1;
};

USTRUCT()
struct FItemComponentProfile {
	GENERATED_BODY()
public:
	TObjectPtr<UPrimitiveComponent> Component;
	bool Visibility;
	bool Simulating;
	//FCollisionResponseContainer CollisionResponses;
	ECollisionEnabled::Type CollisionEnabled;

	FItemComponentProfile() {
	}

	FItemComponentProfile(UPrimitiveComponent* newComp) {
		if (IsValid(newComp)) {
			Component = newComp;
			Visibility = newComp->GetVisibleFlag();
		}
	}

	bool operator == (UPrimitiveComponent* otherComponent) {
		bool val = (Component.Get() == otherComponent);
		return val;
	}
};

USTRUCT(BlueprintType)
struct FInventory2DIndex {
	GENERATED_BODY()
	/*
	* Default constructor generates an invalid coordinate.
	*/
	FInventory2DIndex() {
		X = -1;
		Y = -1;
	}
	/*
	* Auto creates the grid position.
	*/
	FInventory2DIndex(int index, int width) {
		X = index % width;
		Y = index / width;
	}
	/*
	* (0)Left to(N)Right
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Item|Size")
	int X = -1;
	/*
	*  (0)Top to (N)Bottom
	*/
	UPROPERTY(BlueprintReadWrite, Category = "Item|Size")
	int Y = -1;
};

USTRUCT(BlueprintType)
struct FItemDataAmount {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
	int Quantity = 1;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item")
	UInventoryItemData* DataAsset;
};

UENUM(BlueprintType)
enum EItemVisibility {
	BySavedProfile,
	NoVisibility,
	VisibleInRenderTargetsOnly
};

USTRUCT(BlueprintType)
struct FItemCraftingData {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Crafting")
	FName CraftingName = "CraftedItem";
	/*
	 * The amount of this item that is required for this craft.
	 */ 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Crafting")
	int SelfRequiredAmount = 1;
	/*
	 * Amounts of other items required for this craft.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Crafting")
	TArray<FItemDataAmount> RequiredItems;
	/*
	 * The resulting item of this craft and its resulting quantity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Crafting")
	FItemDataAmount ResultingItem;

	FItemCraftingData() 	
	{}

	bool operator == (const FItemCraftingData& otherCraftingData) const {
		return otherCraftingData.CraftingName == CraftingName;
	}

	
};