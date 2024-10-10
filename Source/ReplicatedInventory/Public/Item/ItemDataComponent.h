// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryDataTypes.h"
#include "ItemDataComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuantityChangeSignature, int, oldQuantity, int, newQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FQuantityGeneric);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSizeFlippedSignature, FItemGridSize, oldSize, FItemGridSize, newSize);

class UInventoryItemData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPLICATEDINVENTORY_API UItemDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FQuantityChangeSignature OnQuantityChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FQuantityGeneric OnQuantityDeplete;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FQuantityGeneric OnQuantityFill;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FSizeFlippedSignature OnSizeFlipped;

	UItemDataComponent();

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool SetQuantity(int newAmount);

	UFUNCTION(BlueprintCallable, Category = "Item")
	int RemoveQuantity(int difference);

	UFUNCTION(BlueprintCallable, Category = "Item")
	int AddQuantity(int difference);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item")
	FText GetGridDisplayText() const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetName() const {	return Name; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetDescription() const { return Description; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	UMaterialInstanceDynamic* GetImage() const { return DynamicImage; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemGridSize GetSize() const {	return bItemRotated ? Size.GetFlipped() : Size; }
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetQuantity() const { return Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetMaxQuantity() const { return MaxQuantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetQuantityMaxAddend() const { return FMath::Clamp(MaxQuantity - Quantity, 0, MaxQuantity); }

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool MatchesItem(FName otherItemName) const { return otherItemName == GetName(); }


	UFUNCTION()
	FItemGridSize RotateItem();

	UPROPERTY(EditAnywhere, Category = "Item", BlueprintReadOnly, meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UInventoryItemData> ItemDataAsset;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
private:

	UFUNCTION(Server, Reliable)
	void SetQuantityOnServer(int newQuantity);

	UFUNCTION(NetMulticast, Reliable)
	void BroadcastQuantityUpdate(int oldQuantity, int newQuantity);

	UPROPERTY(EditAnywhere, Category = "Item", Replicated)
	int Quantity = 1;
	
	UPROPERTY()
	FName Name;

	UPROPERTY()
	FName Description;

	UPROPERTY()
	int MaxQuantity = 1;

	UPROPERTY()
	FItemGridSize Size = FItemGridSize(1);
	
	UPROPERTY()
	TObjectPtr<UMaterialInterface> Image;

	UPROPERTY(Replicated)
	bool bItemRotated = false;

	UPROPERTY(Replicated);
	TObjectPtr<UMaterialInstanceDynamic> DynamicImage;
	
	
};
