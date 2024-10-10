// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "InventoryDataTypes.h"
#include "InventoryItemBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInfoChange);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class REPLICATEDINVENTORY_API AInventoryItemBase : public AInfo
{
	GENERATED_BODY()
public:
	// Delegates
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnInfoChange OnInfoChange;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnInfoChange OnDepleted;
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnInfoChange OnRotated;


	AInventoryItemBase();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	class UInventoryItemData* ItemDataAsset;

#pragma region Item Info
public:
	/*
	* Name of the item.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Item Info"); 
	FName Name = FName("Item");
	/*
	* Further describes the item.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Item Info"); 
	FText Description = FText::FromString("Lorem Ipsum");
	/*
	* Shown when hovered over the item.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Item Info"); 
	FText Tooltip = FText::FromString("An Item");
	/*
	* Size of the item in the inventory and how many slots this item takes up.
	*/
	UPROPERTY(Replicated, BlueprintReadOnly, EditAnywhere, Category = "Item Info"); 
	FItemGridSize Size = FItemGridSize(1);
	/*
	* The image that will be displayed in the inventory.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Item Info");
	TObjectPtr<UMaterialInterface> Image = NULL;

	UPROPERTY(BlueprintReadOnly, Category = "Item Info");
	TObjectPtr<UMaterialInstanceDynamic> DynamicImage;
	/*
	* Scalar parameter name on the image material that visually flips the image.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Item Info");
	FName ImageRotateScalarName = FName("IconRotated");
	/*
	 * If this item requires an actor then it is stored here.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Item Info")
	TObjectPtr<AActor> ItemPayload;

private:
	/*
	* Maximum Quantity that can be inside a slot. If this is set to <=0 then this is effectively infinite.
	*/
	UPROPERTY(EditAnywhere, Category = "Item Info");
	int MaxQuantity = 1;
	/*
	* Amount of the item that is currently in this slot.
	*/
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_Quantity)
	int Quantity = 1;

	/*
	* Determines if the item is rotated.
	*/
	UPROPERTY(ReplicatedUsing = OnRep_bRotated);
	bool bRotated = false;

#pragma endregion

#pragma region Functions
public:
	/*
	* The text that is displayed for the item in the grid in the slot's corner. Normally this would be the amount of an item a slot has.
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FText GetGridText() const;
	/*
	* Created to be overridden in child C++ classes. (By default return quantity)
	*/
	UFUNCTION()
	virtual FText GetGridText_Internal() const;
	/*
	* Returns the amount of an item.
	*/
	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetQuantity() const;
	/*
	* Returns the maximum quantity 
	*/
	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetMaxQuantity() const;
	/*
	* Returns MaxQuantity - Quantity
	*/
	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetQuantityDifference() const { return MaxQuantity - Quantity; }
	/*
	* Sets the Quantity. Returns excess that was not used.
	*/
	UFUNCTION(BlueprintCallable, Category = "Item")
	int SetQuantity(EItemModify modification, int value);
	/*
	* Rotates the item. Swaps the Size.Width and Size.Height vars.
	*/
	UFUNCTION()
	void Rotate();
	/*
	* Returns true if the item is rotated.
	*/
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool GetRotated() const { return bRotated; }

protected:
	/*
	* Called when item information is changed. Initially only called when Quantity is changed or when item is rotated.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Item")
	void OnDeplete();

private:
	/*
	Rotates the item on the server for replication.
	*/
	UFUNCTION(Server, Reliable)
	void Rotate_Server();
	/*
	* RepNotify for bRotated. Used for Material scalar change.
	*/
	UFUNCTION()
	void OnRep_bRotated(bool previousValue);
	/*
	* Sets the quantity on the server for replication.
	*/
	UFUNCTION(Server, Reliable)
	void SetQuantity_Server(EItemModify modification, int value);
	/*
	* Replicates an info change broadcast when a variable is modified. Should be called from server.
	*/
	UFUNCTION(NetMulticast, Reliable)
	void ReplicateInfoChange();
	/*
	* Replicates depletion when quantity hits 0. Should be called from server.
	*/
	UFUNCTION(NetMulticast, Reliable)
	void ReplicateDepleted();
	/*
	* Ensures that update is called after the value is set on the clients.
	*/
	UFUNCTION()
	void OnRep_Quantity(int oldVale);


#pragma endregion
};
