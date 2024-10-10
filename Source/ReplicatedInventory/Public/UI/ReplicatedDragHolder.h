// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ReplicatedDragHolder.generated.h"

class UItemDataComponent;

UCLASS()
class REPLICATEDINVENTORY_API AReplicatedDragHolder : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AReplicatedDragHolder();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Replicate vars.
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

public:	
	UPROPERTY()
	APlayerController* OwningPlayer;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Inventory|Item")
	bool bPreviewRotated = false;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void RotateHeldItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void HoldItem(UItemDataComponent* item, int index);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void ReturnItem();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	UItemDataComponent* RetrieveItem(bool bDestroyHolderOnRetrieve = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	UItemDataComponent* CheckItem() const { return HoldingItem; }

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory|Item")
	TObjectPtr<class UInventoryComponent> OriginalInventory = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Item")
	void BindReturnToDragCancel(UDragDropOperation* InOperation);

	UFUNCTION()
	void BoundedReturnItemToInventory(UDragDropOperation* Operation);


private:
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory|Item")
	int OriginalIndex = -1;

	UPROPERTY(Replicated, VisibleAnywhere, Category = "Inventory|Item")
	TObjectPtr<UItemDataComponent> HoldingItem = nullptr;

	UFUNCTION()
	void UpdateItemInfo(UItemDataComponent* itemData, int index);

	UFUNCTION(Server, Reliable, Category = "Inventory|Item")
	void HoldItem_Server(UItemDataComponent* itemData, int index);

	UFUNCTION(Server, Reliable, Category = "Inventory|Item")
	void ReturnItem_Server();

	UFUNCTION(Server, Reliable, Category = "Inventory|Item")
	void RetrieveItem_Server();

	UFUNCTION(Server, Reliable, Category = "Inventory|Item")
	void RotateReplicatedItem_Server();

	UFUNCTION()
	void Cleanup();

};
