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
	FCollisionResponseContainer CollisionResponses;
	ECollisionEnabled::Type CollisionEnabled;

	FItemComponentProfile() {
	}

	FItemComponentProfile(UPrimitiveComponent* newComp) {
		if (IsValid(newComp)) {
			Component = newComp;
			Visibility = newComp->GetVisibleFlag();
			Simulating = newComp->IsSimulatingPhysics();
			CollisionResponses = newComp->GetCollisionResponseToChannels();
			CollisionEnabled = newComp->GetCollisionEnabled();
		}
	}

	bool operator == (UPrimitiveComponent* otherComponent) {
		return Component == otherComponent;
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