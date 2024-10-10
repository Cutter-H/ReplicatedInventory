// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "InventoryDataTypes.h"
#include "InventorySlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemUpdatedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemQuantityChangeSignature, int, oldQuantity, int, newQuantity);

class UInventoryComponent;
class IInventoryInterface;
class UItemDataComponent;

/**
 * This class stands in as an interface between the inventory and the item. Due to this, items are not dependent upon the inventory and may be added anywhere.
 */
UCLASS(BlueprintType)
class REPLICATEDINVENTORY_API AInventorySlot : public AInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory|Slot")
	FOnItemUpdatedSignature OnSlotUpdated;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory|Slot")
	FOnItemQuantityChangeSignature OnItemQuantityChange;
	
	AInventorySlot();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	UFUNCTION()
	void AttachToInventory(UInventoryComponent* inventory, int index);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	UInventoryComponent* GetInventory() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	int GetIndex() const { return InventoryIndex; }

	// There are no checks here. If this is called without proper empty checks for space then issues may occur.
	UFUNCTION()
	bool SetItemInfo(AActor* item);
	
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	FName GetName() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	UItemDataComponent* GetItemInfo() const { return ItemDataComp; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	AActor* GetItem() const;


	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	FItemGridSize GetSize() const;

	UFUNCTION()
	AActor* RemoveItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	bool RotateItem();


	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	void DestroyItem();

	UFUNCTION()
	void OnInventoryWidthChange(int increaseAmount);

	/*
	* Detects if the item in this slot is the same as the given item.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	bool MatchesItem(AActor* otherItem) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Slot")
	bool IsTaken() const { return bSlotTaken; }


private:
	UFUNCTION()
	void SetSlotTaken();

	UFUNCTION()
	void SetSlotEmpty();

	UFUNCTION(Server, Reliable)
	void SetSlotTakenStatus_Server(bool newTaken);

	UFUNCTION(Server, Reliable)
	void AttachToInventory_Server(UInventoryComponent* inventory, int index);

	UFUNCTION(Server, Reliable)
	void UpdateItem_Server(AActor* item, UItemDataComponent* dataComp);

	UFUNCTION(NetMulticast, Reliable)
	void ReplicateSlotUpdated();

	UFUNCTION()
	void SetItemPhysicality(bool visible, bool bOnlyVisibleInRenderTargets = false);

	UFUNCTION(NetMulticast, Reliable)
	void ReplicateItemPhysicality(bool visible, bool bOnlyVisibleInRenderTargets);

	UFUNCTION()
	void OnItemSelfDestroy(AActor* destroyedActor);

	UPROPERTY(Replicated, VisibleAnywhere)
	int InventoryIndex = -1; 
	UPROPERTY(Replicated, VisibleAnywhere)
	TObjectPtr<UInventoryComponent> Inventory;
	UPROPERTY(Replicated, VisibleAnywhere)
	bool bSlotTaken = false;

	TArray<FItemComponentProfile> ItemProfile;
	UPROPERTY(Replicated)
	TObjectPtr<UItemDataComponent> ItemDataComp;

};
