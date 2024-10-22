// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InventoryDataTypes.h"
#include "InventoryItemData.generated.h"

/**
 * 
 */
UCLASS()
class REPLICATEDINVENTORY_API UInventoryItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/*
	 * Unique Name of the item. This will be used to check if items match.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName Name;
	/*
	 * Description used for the item.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName Description;
	/*
	 * How many of this item can be in a single stack.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	int MaxQuantity = 1;
	/*
	 * How much room in the inventory does this item take up.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FItemGridSize Size;
	/*
	 * This is the name of the Scalar parameter used to rotate the item's image.
	 * This will be toggled between 0 and 1 when rotating the item.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	FName ImageRotateScalarName = FName("IconRotated");
	/*
	 * The image shown in the inventory grid.	
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	TObjectPtr<UMaterialInterface> Image;
	/* 
	 * A reference to the actor normally used to physically portray this item. If an actor needs to be spawned (such as at a vendor) it will check this.
	 * (NEEDS to be set.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Data")
	TSubclassOf<AActor> ItemClass;
	/*
	 * Activation options for the item. This can be anything from widgets or GameplayAbilities, hence the subclass of Object.
	 * There exists a Getter on the ItemDataComponent class for these.
	 */
	TArray<TSubclassOf<UObject>> ActivationOptions;
};
