// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemActorBase.generated.h"

UENUM(BlueprintType)
enum EItemActorSpawnChoice
{
	DataAsset,
	ItemClass
};

UCLASS()
class REPLICATEDINVENTORY_API AItemActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItemActorBase();

	UStaticMeshComponent* Mesh;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Replicated Inventory|Item")
	class AInventoryItemBase* Item;
	/*
	* Sets the item via another item.
	* Returns an item if one is already present.
	*/
	UFUNCTION(BlueprintCallable, Category = "Replicated Inventory|Item")
	class AInventoryItemBase* SetItemInfo(class AInventoryItemBase* newItem);
	/*
	* Sets the item via Data Asset.
	* Returns an item if one is already present.
	*/
	UFUNCTION(BlueprintCallable, Category = "Replicated Inventory|Item")
	class AInventoryItemBase* SetItemByDataAsset(class UInventoryItemData* newItemDataAsset, int itemQuantityOverride = 0);
	/*
	* Sets the item via by Item Class.
	* Returns an item if one is already present.
	*/
	UFUNCTION(BlueprintCallable, Category = "Replicated Inventory|Item")
	class AInventoryItemBase* SetItemByClass(TSubclassOf<class AInventoryItemBase> newItemClass, int itemQuantityOverride = 1);

	UFUNCTION(Server, Reliable)
	void CreateItemAndAssign(TSubclassOf<class AInventoryItemBase> newItemClass, int itemQuantity);

	UFUNCTION(Server,Reliable)
	void AssignItem(class AInventoryItemBase* newItem);

	UFUNCTION(NetMulticast, Reliable)
	void UpdateMesh();

protected:
	/*
	* Selects how to spawn the item.
	*/
	EItemActorSpawnChoice SpawnFrom = EItemActorSpawnChoice::DataAsset;
	/*
	* Data Asset that the Item is spawned from.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Replicated Inventory|Spawning", meta = (EditCondition = "SpawnFrom == EItemActorSpawnChoice::DataAsset", EditConditionHides))
	TObjectPtr<class UInventoryItemData> DataAsset;
	/*
	* The class of the item that this actor is bound to.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Replicated Inventory|Spawning", meta = (EditCondition = "SpawnFrom == EItemActorSpawnChoice::ItemClass", EditConditionHides))
	TSubclassOf<class AInventoryItemBase> ItemClass;
	/*
	* Overrides the quantity from the Item Class and/or Data Asset.
	* If this is below 1 then the quantity is not overriden.
	* If the Item Class is used then this should be set. If it isn't the actor is destroyed.
	*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Replicated Inventory|Spawning")
	int QuantityOverride = 0;
};
