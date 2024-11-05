// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryDataTypes.h"
#include "Item/InventoryItemData.h"
#include "ItemDataComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuantityChangeSignature, int, oldQuantity, int, newQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FItemGenericSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemGridTextChangeSignature, FText, newGridText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSizeFlippedSignature, FItemGridSize, oldSize, FItemGridSize, newSize);

class UInventoryItemData;

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPLICATEDINVENTORY_API UItemDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Item")
	FQuantityChangeSignature OnQuantityChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Item")
	FItemGenericSignature OnQuantityDeplete;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Item")
	FItemGenericSignature OnQuantityFill;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Item")
	FSizeFlippedSignature OnSizeFlipped;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Item")
	FItemGridTextChangeSignature OnGridTextChange;

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
	FName GetItemName() const {	return IsValid(ItemDataAsset) ? ItemDataAsset->Name : FName(""); }

	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetDescription() const { return IsValid(ItemDataAsset) ? ItemDataAsset->Description : FName(""); }

	UFUNCTION(BlueprintCallable, Category = "Item")
	UMaterialInstanceDynamic* GetImage() const { return DynamicImage; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemGridSize GetSize() const {	return IsValid(ItemDataAsset) ? bItemRotated ? ItemDataAsset->Size.GetFlipped() : ItemDataAsset->Size : FItemGridSize(); }
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetQuantity() const { return Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetMaxQuantity() const { return IsValid(ItemDataAsset) ? ItemDataAsset->MaxQuantity : 1; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	int GetQuantityMaxAddend() const { return IsValid(ItemDataAsset) ? FMath::Clamp(ItemDataAsset->MaxQuantity - Quantity, 0, ItemDataAsset->MaxQuantity) : 0; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool MatchesItem(FName otherItemName) const { return IsValid(ItemDataAsset) ? otherItemName == ItemDataAsset->Name : false; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	FItemGridSize RotateItem();

	UFUNCTION(BlueprintCallable, Category = "Item")
	FName GetItemRotationScalarName() const { return IsValid(ItemDataAsset) ? ItemDataAsset->ImageRotateScalarName : FName(""); }

	UFUNCTION(NetMulticast, Reliable)
	void ReplicateDataAssetInfo(UInventoryItemData* newData);

	UFUNCTION()
	bool HasAuthority() const;

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = "Item")
	void UpdatePrimitiveProfile(bool overrideOriginals = false);

	UFUNCTION()
	void SetItemDataAsset(UInventoryItemData* newItemData);

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Item")
	void SetItemVisibility(EItemVisibility newVisibility);

	UFUNCTION(BlueprintCallable, Category = "Item|Activation")
	TArray<TSubclassOf<UObject>> GetActivationOptions() const { return IsValid(ItemDataAsset) ? ItemDataAsset->ActivationOptions : TArray<TSubclassOf<UObject>>(); }

	UFUNCTION(BlueprintCallable, Category = "Item|Crafting")
	TArray<FItemCraftingData> GetCraftingOptions() const { return IsValid(ItemDataAsset) ? ItemDataAsset->CraftingOptions : TArray<FItemCraftingData>(); }

	UPROPERTY(EditAnywhere, Category = "Item", BlueprintReadOnly, meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UInventoryItemData> ItemDataAsset;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
private:	
	UFUNCTION(Server, Reliable)
	void RotateItemOnServer();

	UFUNCTION(NetMulticast, Reliable)
	void ReplicateItemRotation(bool newRotated, FItemGridSize newSize);

	UFUNCTION(Server, Reliable)
	void SetQuantityOnServer(int newQuantity);

	UFUNCTION(NetMulticast, Reliable)
	void BroadcastQuantityUpdate(int oldQuantity, int newQuantity);

	UFUNCTION()
	void OnRep_Quantity(int oldQuantity);

	UFUNCTION()
	int ProfileContainsComp(UPrimitiveComponent* comp) const {
		for (int i = 0; i < PrimitiveProfile.Num(); i++) {
			if (PrimitiveProfile[i].Component == comp) {
				return i;
			}
		} return -1;
	}

	UPROPERTY(EditAnywhere, Category = "Item", Replicated, ReplicatedUsing = "OnRep_Quantity")
	int Quantity = 1;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	bool bDestroyOnDepletion = true;	
	
	UPROPERTY(Replicated)
	bool bItemRotated = false;

	UPROPERTY();
	TObjectPtr<UMaterialInstanceDynamic> DynamicImage;
	
	UPROPERTY(Replicated)
	TArray<FItemComponentProfile> PrimitiveProfile;
};
