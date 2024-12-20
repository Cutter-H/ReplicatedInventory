// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryDataTypes.h"
#include "InventoryComponent.generated.h"

class UInventoryItemData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGeneralInventoryChangeSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventorySizeChangeSignature, int, delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventorySlotChangeSignature, int, slot, UItemDataComponent*, newItem, EInventorySlotState, newSlotState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBroadcastInventorySignature, UInventoryComponent*, inventory);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInventorySlotRotatedSignature, int, slot, FItemGridSize, newSize);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable, BlueprintType )
class REPLICATEDINVENTORY_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/*
	* Sets default values for this component's properties
	*/ 
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnGeneralInventoryChangeSignature OnInventoryChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnBroadcastInventorySignature OnInventoryGenerated;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventorySizeChangeSignature OnInventorySizeChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventorySizeChangeSignature OnInventoryWidthChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventorySlotChangeSignature OnInventorySlotChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Inventory")
	FOnInventorySlotRotatedSignature OnInventorySlotRotated;


protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	

public:	
	/*
	* Called every frame
	*/ 
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/*
	* Returns all slots that are in the inventory.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<UItemDataComponent*> GetAllItems() const { return ItemSlots; };
	/*
	* Returns empty if a slot is out of bounds (Too far right or past length).
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TArray<int> GetSlots(int index, FItemGridSize size, bool bIncludeOrigin = false) const;
	/*
	* Returns the size of the inventory.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetInventorySize() const { return InventorySize; }
	/*
	* Returns how wide the inventory grid is.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetInventoryWidth() const { return InventoryWidth; }
	/*
	* Returns the visual height of the inventory.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetInventoryHeight() const { return (InventorySize / InventoryWidth) + ((InventorySize%InventoryWidth > 0) ? 1 : 0); }
	/*
	* Returns true if all given slots at given indexes are empty.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SlotsAreEmpty(TArray<int> indexes) const;
	/*
	* Returns true if all given slots at given indexes are empty.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SlotIsEmpty(int index) const;
	/*
	* Converts the position from the 2D grid to an index.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int Index2DToInt(FInventory2DIndex index2D) const;
	/*
	* Converts a given index to the position in the 2D grid.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FInventory2DIndex IndexIntTo2D(int index) const;
	/*
	* Increases the inventory size.
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Inventory")
	void IncreaseInventorySize(int increaseAmount);
	/*
	* Increases the inventory's width. Keep in mind this will effectively increase the size of the inventory by the truncated int of (Size/OriginalWidth).
	* This will not extend the final row.
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Inventory")
	bool IncreaseInventoryWidth(int increaseAmount);
	/*
	* Adds an item to the inventory. If any excess exists then the input item will be modified.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	int AddItemToInventory(AActor* item, int desiredIndex = -1);

	/*
	 * Adds item data to the inventory. If any excess exists then the input item will be modified.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	int AddItemComponentToInventory(UItemDataComponent* itemData, int desiredIndex = -1);
	/*
	* Adds an item to the inventory using a data asset. May create a new item if the quantity is not added with existing items.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	int AddItemToInventoryUsingData(const FItemDataAmount& item, FItemDataAmount& modifiedItemData, int desiredIndex = -1);
	/*
	* Removes and destroys the item at the given index.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	void DestroyItemAtIndex(int index = -1);
	/*
	* Removes the item from the index and returns it.
	*/
	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	AActor* RemoveItem(int index = -1);
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	void OnFinishGeneratingInventory(UInventoryComponent* inventory);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	UItemDataComponent* GetItem(int index) const { return ItemSlots.IsValidIndex(index) ? ItemSlots[index] : NULL; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsSlotTaken(int index) const { return TakenSlots.Contains(index); }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool CanRotateSlot(int index) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RotateSlot(int index);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasRoomForItem(UItemDataComponent* item, int numberOfItems = 1, int desiredIndex = -1) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int GetNumOfItem(FName itemName) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	TArray<FItemCraftingData> GetCraftableOptions(int index) const;

	//UFUNCTION(BlueprintCallable, Category = "Inventory|Crafting")
	//bool CraftItem(int index, FItemCraftingData craftingData);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void DestroyNumOfItem(FName itemName, int numberToDestroy, int startingIndex = -1);

private:
	/*
	 * Sets the item at the given index. More than just assigning needs to be done.
	 */
	UFUNCTION(Server, Reliable)
	void SetItemOnServer(int index, UItemDataComponent* item);
	UFUNCTION()
	void SetItem(int index, UItemDataComponent* item);
	/*
	* Utility function for internal use. This behaves similarly to AActor's HasAuthority() function, but also returns true if the owning actor is not replicated.. 
	*/
	UFUNCTION()
	bool HasAuthority() const;
	/*
	* Returns true if the owner is a locally controlled pawn.
	*/
	UFUNCTION()
	bool IsLocallyControlled() const;
	/*
	* Increases the inventory size on server for replication.
	*/
	UFUNCTION(Server, Reliable)
	void IncreaseInventorySize_Server(int amount);
	/*
	* Increases the inventory Width on server for replication.
	*/
	UFUNCTION(Server, Reliable)
	void IncreaseInventoryWidth_Server(int amount);
	/*
	* Server helper of AddItemToInventoryUsingData. This is used for creating the replicated Item. If a new item is needed, the functionality is finished up here.
	*/
	UFUNCTION(Server, Reliable)
	void AddItemToIndexWithData_Server(UInventoryItemData* itemData, int quantity, int desiredIndex);
	
	UFUNCTION()
	AActor* GenerateItemWithData(UInventoryItemData* itemData, int quantity);
	/*
	* Notifies that the inventory has been generated on this client. Calls OnFinishGeneratingInentory and broadcasts OnInventoryGenerated on all clients.
	*/
	UFUNCTION(NetMulticast, Reliable)
	void ReplicateFinishedGeneratingInventory_Multi();
	/*
	* Notifies the server that the inventory has been generated on this client. This is used in-case the client has a slow start.
	*/
	UFUNCTION(Server, Reliable)
	void RequestFinishGeneratingInventory_Server();
	/*
	 * Checks if a given index is within boundries.
	 */
	UFUNCTION()
	bool IsValidIndex(int index) const { return (index >= 0) || (index < InventorySize); }

	UFUNCTION(NetMulticast, Reliable)
	void UpdatedIndex_Multi(int index, UItemDataComponent* item, EInventorySlotState newSlotState);

	UFUNCTION()
	void OnRep_ItemSlots(TArray<UItemDataComponent*> oldItemSlots);

	UFUNCTION(NetMulticast, Reliable)
	void ReplicateTakenSlotChange_Multi(int index, bool taken);

	UFUNCTION(Server, Reliable)
	void UpdateTakenSlotChange_Server(int index, bool taken);

	UFUNCTION(Server, Reliable)
	void RotateItemOnServer(int index);

	UFUNCTION(NetMulticast, Reliable)
	void ReplicateSlotRotated_Multi(int index, FItemGridSize newSize);

	/*
	* The slots of the items stored in this inventory.
	*/
	UPROPERTY(Replicated, ReplicatedUsing="OnRep_ItemSlots")
	TArray<TObjectPtr<UItemDataComponent>> ItemSlots;
	/*
	* How many slots can fit into this inventory.
	*/
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	int InventorySize = 1;
	/*
	* The width of the 2D array.
	*/
	UPROPERTY(Replicated, EditAnywhere, Category = "Inventory")
	int InventoryWidth = 1;

	UPROPERTY(Replicated)
	TArray<int> TakenSlots;
};
