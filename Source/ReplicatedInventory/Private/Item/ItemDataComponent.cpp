// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemDataComponent.h"
#include "Item/InventoryItemData.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UItemDataComponent::UItemDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

}

bool UItemDataComponent::SetQuantity(int newAmount) {
	if (newAmount == Quantity || !IsValid(GetOwner())) {
		return false;
	}
	if (GetOwner()->HasAuthority() || !GetOwner()->GetIsReplicated()) {
		int old = Quantity;
		Quantity = newAmount;
		if (GetOwner()->GetIsReplicated()) {
			BroadcastQuantityUpdate(old, newAmount);
		}
		else {
			OnQuantityChange.Broadcast(old, newAmount);
			if (newAmount == 0) {
				OnQuantityDeplete.Broadcast();
			}
			if (newAmount == MaxQuantity) {
				OnQuantityFill.Broadcast();
			}
		}
	}
	else {
		SetQuantityOnServer(newAmount);
	}
	return true;
}

int UItemDataComponent::RemoveQuantity(int difference) {
	if (difference < 0) {
		return -1 * AddQuantity(difference *= -1);
	}
	if (difference == 0) {
		return 0;
	}
	int retVal = 0;
	if (difference < Quantity) {
		retVal = Quantity - difference;
	}
	if (!SetQuantity(FMath::Clamp(Quantity - difference, 0, MaxQuantity))) {
		return difference;
	}
	return retVal;
}

int UItemDataComponent::AddQuantity(int difference) {
	if (difference < 0) {
		return -1 * RemoveQuantity(difference *= -1);
	}
	if (difference == 0) {
		return 0;
	}
	
	int retVal = 0;
	if (difference > (MaxQuantity - Quantity)) {
		retVal = difference - (MaxQuantity - Quantity);
	}
	if (!SetQuantity(FMath::Clamp(Quantity + difference, 0, MaxQuantity))) {
		return difference;
	}
	return retVal;
}

FText UItemDataComponent::GetGridDisplayText_Implementation() const {
	return FText::FromString(FString::FromInt(Quantity));
}


FItemGridSize UItemDataComponent::RotateItem() {
	bItemRotated = !bItemRotated;

	return GetSize();
}

// Called when the game starts
void UItemDataComponent::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(ItemDataAsset)) {
		Name = ItemDataAsset->Name;
		Description = ItemDataAsset->Description;
		MaxQuantity = ItemDataAsset->MaxQuantity;
		Size = ItemDataAsset->Size;
		Image = ItemDataAsset->Image;
	}
	if (IsValid(Image)) {
		DynamicImage = UMaterialInstanceDynamic::Create(Image, this);

	}
}

void UItemDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UItemDataComponent, Quantity);
	DOREPLIFETIME(UItemDataComponent, DynamicImage);
}

void UItemDataComponent::SetQuantityOnServer_Implementation(int newQuantity) {
	int old = Quantity;
	Quantity = newQuantity;
	BroadcastQuantityUpdate(old, newQuantity);
}

void UItemDataComponent::BroadcastQuantityUpdate_Implementation(int oldQuantity, int newQuantity) {
	OnQuantityChange.Broadcast(oldQuantity, newQuantity);
	if (newQuantity == 0) {
		OnQuantityDeplete.Broadcast();
	}
	if (newQuantity == MaxQuantity) {
		OnQuantityFill.Broadcast();
	}
	OnGridTextChange.Broadcast();
}

