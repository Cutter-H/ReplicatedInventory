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
			OnGridTextChange.Broadcast(GetGridDisplayText());
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
	return Quantity > 1 ? FText::FromString(FString::FromInt(Quantity)) : FText::FromString("");
}
FItemGridSize UItemDataComponent::RotateItem() {
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, FString("Started Rotating Item."));
	FItemGridSize retVal = GetSize().GetFlipped();
	if (HasAuthority()) {
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, FString("Initial call was on server."));
		bItemRotated = !bItemRotated;
		if (DynamicImage) {
			DynamicImage->SetScalarParameterValue(ItemRotationScalar, bItemRotated ? 1.f : 0.f);
		}
	}
	else {
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, FString("Initial call was NOT on server."));
		RotateItemOnServer();
	}
	return retVal;
}
void UItemDataComponent::ReplicateDataAssetInfo_Implementation(UInventoryItemData* newData) {
	ItemDataAsset = newData;
	if (IsValid(ItemDataAsset)) {
		Name = ItemDataAsset->Name;
		Description = ItemDataAsset->Description;
		MaxQuantity = ItemDataAsset->MaxQuantity;
		Size = ItemDataAsset->Size;
		Image = ItemDataAsset->Image;
		ItemRotationScalar = ItemDataAsset->ImageRotateScalarName;
		DynamicImage = UMaterialInstanceDynamic::Create(Image, this);
	}
}
bool UItemDataComponent::HasAuthority() const {
	if (!IsValid(GetOwner()->GetOwner())) {
		return false;
	}
	if (!GetOwner()->GetOwner()->GetIsReplicated()) {
		return true;
	}
	return GetOwner()->GetOwner()->GetLocalRole() == ROLE_Authority;
}

void UItemDataComponent::BeginPlay() {
	Super::BeginPlay();
	if (IsValid(ItemDataAsset)) {
		Name = ItemDataAsset->Name;
		Description = ItemDataAsset->Description;
		MaxQuantity = ItemDataAsset->MaxQuantity;
		Size = ItemDataAsset->Size;
		Image = ItemDataAsset->Image;
		ItemRotationScalar = ItemDataAsset->ImageRotateScalarName;
		DynamicImage = UMaterialInstanceDynamic::Create(Image, this);
	}
}
void UItemDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UItemDataComponent, Quantity);
	DOREPLIFETIME_CONDITION_NOTIFY(UItemDataComponent, bItemRotated, COND_None, REPNOTIFY_Always);
}

void UItemDataComponent::RotateItemOnServer_Implementation() {
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 1.f, FColor::Green, FString("Initial call was sent to the server."));
	bItemRotated = !bItemRotated;
	if (DynamicImage) {
		DynamicImage->SetScalarParameterValue(ItemRotationScalar, bItemRotated ? 1.f : 0.f);
	}
	OnSizeFlipped.Broadcast(GetSize().GetFlipped(), GetSize());
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
	OnGridTextChange.Broadcast(GetGridDisplayText());
}
void UItemDataComponent::OnRep_ItemRotated() {
	if (DynamicImage) {
		DynamicImage->SetScalarParameterValue(ItemRotationScalar, bItemRotated ? 1.f : 0.f);
	}
	FItemGridSize oldSize = GetSize().GetFlipped();
	FItemGridSize newSize = GetSize();
	OnSizeFlipped.Broadcast(GetSize().GetFlipped(), GetSize());
}