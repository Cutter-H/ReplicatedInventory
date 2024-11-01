// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemDataComponent.h"
#include "Item/InventoryItemData.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UItemDataComponent::UItemDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

}
bool UItemDataComponent::SetQuantity(int newAmount) {
	int correctedNewAmount = FMath::Clamp(newAmount, 0, MaxQuantity);
	if (correctedNewAmount == Quantity || !IsValid(GetOwner())) {
		return false;
	}
	if (HasAuthority()) {
		int old = Quantity;
		Quantity = correctedNewAmount;
				
		OnQuantityChange.Broadcast(old, correctedNewAmount);
		if (correctedNewAmount <= 0) {
			OnQuantityDeplete.Broadcast();
		}
		if (correctedNewAmount >= MaxQuantity) {
			OnQuantityFill.Broadcast();
		}
		OnGridTextChange.Broadcast(GetGridDisplayText());
		
	}
	else {
		SetQuantityOnServer(correctedNewAmount);
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
	FItemGridSize retVal = GetSize().GetFlipped();
	if (HasAuthority()) {
		if (!GetOwner()->GetIsReplicated()) {
			bItemRotated = !bItemRotated;
			if (DynamicImage) {
				DynamicImage->SetScalarParameterValue(ItemRotationScalar, bItemRotated ? 1.f : 0.f);
			}
			OnSizeFlipped.Broadcast(GetSize().GetFlipped(), GetSize());
			return retVal;
		}
		ReplicateItemRotation(!bItemRotated, retVal);
	}
	else {
		RotateItemOnServer();
	}
	return retVal;
}
void UItemDataComponent::ReplicateDataAssetInfo_Implementation(UInventoryItemData* newData) {
	ItemDataAsset = newData;
	UpdateData();
}
void UItemDataComponent::UpdateData() {
	if (IsValid(ItemDataAsset)) {
		Name = ItemDataAsset->Name;
		Description = ItemDataAsset->Description;
		MaxQuantity = ItemDataAsset->MaxQuantity;
		Size = ItemDataAsset->Size;
		Image = ItemDataAsset->Image;
		ItemRotationScalar = ItemDataAsset->ImageRotateScalarName;
		DynamicImage = UMaterialInstanceDynamic::Create(Image, this);
		ActivationOptions = ItemDataAsset->ActivationOptions;
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
void UItemDataComponent::UpdatePrimitiveProfile(bool overrideOriginals) {
	TArray<UPrimitiveComponent*> comps;
	GetOwner()->GetComponents<UPrimitiveComponent>(comps);
	for (int i = 0; i < PrimitiveProfile.Num(); i++) {
		if (!IsValid(PrimitiveProfile[i].Component)) {
			PrimitiveProfile.RemoveAt(i);
		}
	}
	for (UPrimitiveComponent* comp : comps) {
		int foundIndex = ProfileContainsComp(comp);
		if (foundIndex >= 0 && overrideOriginals) {
			PrimitiveProfile[foundIndex] = FItemComponentProfile(comp);
		}
		else {
			if (foundIndex < 0) {
				PrimitiveProfile.Add(FItemComponentProfile(comp));
			}
		}
	}
}
void UItemDataComponent::SetItemVisibility(EItemVisibility newVisibility) {
	for (int i = 0; i < PrimitiveProfile.Num(); i++) {
		FItemComponentProfile settings = PrimitiveProfile[i];
		UPrimitiveComponent* comp = settings.Component;
		comp->SetVisibility(newVisibility != EItemVisibility::NoVisibility ? settings.Visibility : false);
		comp->SetVisibleInSceneCaptureOnly(newVisibility == EItemVisibility::VisibleInRenderTargetsOnly);
	}
}

TArray<TSubclassOf<UObject>> UItemDataComponent::GetActivationOptions() const {
	return IsValid(ItemDataAsset) ? ItemDataAsset->ActivationOptions : TArray<TSubclassOf<UObject>>();
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
	if (HasAuthority()) {
		UpdatePrimitiveProfile();
	}
}
void UItemDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UItemDataComponent, Quantity, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME(UItemDataComponent, bItemRotated);
	DOREPLIFETIME(UItemDataComponent, PrimitiveProfile);
}

void UItemDataComponent::RotateItemOnServer_Implementation() {
	ReplicateItemRotation(!bItemRotated, GetSize().GetFlipped());
}
void UItemDataComponent::ReplicateItemRotation_Implementation(bool newRotated, FItemGridSize newSize) {
	bItemRotated = newRotated;
	if (DynamicImage) {
		DynamicImage->SetScalarParameterValue(ItemRotationScalar, newRotated ? 1.f : 0.f);
	}
	OnSizeFlipped.Broadcast(newSize.GetFlipped(), newSize);
}
void UItemDataComponent::SetQuantityOnServer_Implementation(int newQuantity) {
	int old = Quantity;
	Quantity = newQuantity;
	OnQuantityChange.Broadcast(old, newQuantity);
	if (Quantity <= 0) {
		OnQuantityDeplete.Broadcast();
	}
	if (Quantity >= MaxQuantity) {
		OnQuantityFill.Broadcast();
	}
	OnGridTextChange.Broadcast(GetGridDisplayText());
	if (newQuantity <= 0) {
		GetOwner()->Destroy(true);
	}
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
void UItemDataComponent::OnRep_Quantity(int oldQuantity) {
	OnQuantityChange.Broadcast(oldQuantity, Quantity);
	if (Quantity <= 0) {
		OnQuantityDeplete.Broadcast();
	}
	if (Quantity >= MaxQuantity) {
		OnQuantityFill.Broadcast();
	}
	OnGridTextChange.Broadcast(GetGridDisplayText());
}