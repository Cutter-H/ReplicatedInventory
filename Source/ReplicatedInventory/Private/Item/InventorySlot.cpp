// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/InventorySlot.h"
/*/
#include "Item/ItemDataComponent.h"

#include "InventoryInterface.h"
#include "InventoryComponent.h"

#include "UI/ReplicatedDragHolder.h"
#include "Net/UnrealNetwork.h"


AInventorySlot::AInventorySlot() {
	bReplicates = true;
	bAlwaysRelevant = true;

}

void AInventorySlot::BeginPlay() {
	Super::BeginPlay();

}

void AInventorySlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInventorySlot, ItemDataComp);
	DOREPLIFETIME(AInventorySlot, InventoryIndex);
	DOREPLIFETIME(AInventorySlot, Inventory);
	DOREPLIFETIME(AInventorySlot, bSlotTaken);
}

void AInventorySlot::AttachToInventory(UInventoryComponent* inventory, int index) {
	if (IsValid(Inventory)) return;
	if (HasAuthority()) {
		Inventory = inventory;
		InventoryIndex = index;
	}
	else
		AttachToInventory_Server(inventory, index);
}
UInventoryComponent* AInventorySlot::GetInventory() const {
	if (IsValid(Inventory))
		return Inventory;
	else
		return NULL;
}

bool AInventorySlot::SetItemInfo(AActor* item)
{
	UItemDataComponent* dataComp = item->GetComponentByClass<UItemDataComponent>();
	if (!IsValid(Inventory) || (IsValid(item) && !IsValid(dataComp))) {
		return false;
	}
	
	if (IsValid(dataComp) && IsValid(item)) {
		TArray<AInventorySlot*> slots = Inventory->GetSlots(InventoryIndex, dataComp->GetSize(), true);
		if (slots.Num() <= 0) {
			return false;
		}
		if (HasAuthority()) {
			RemoveItem();
			for (AInventorySlot* slot : slots) {
				if (slot == this) {
					ItemDataComp = dataComp;
					ItemProfile.Empty();
					for (UActorComponent* comp : item->GetComponents()) {
						if (UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(comp)) {
							ItemProfile.Add(prim);
						}
					}
					SetItemPhysicality(false, false);
				}
				else {
					slot->SetSlotTaken();
				}
			}
		}
		else {
			UpdateItem_Server(item, dataComp);
		}
		
	}
	else{
		TArray<AInventorySlot*> slots = Inventory->GetSlots(InventoryIndex, dataComp->GetSize(), true);
		if (slots.Num() <= 0) {
			return false;
		}
		if (HasAuthority()) {
			RemoveItem();
			for (AInventorySlot* slot : slots) {
				slot->SetSlotEmpty();
			}
		}
		else {
			UpdateItem_Server(nullptr, nullptr);
		}
		
	}
	if (HasAuthority()) {
		OnSlotUpdated.Broadcast();
		ReplicateSlotUpdated();
	}
	return true;
}
FName AInventorySlot::GetItemName() const {
	return IsValid(ItemDataComp) ? ItemDataComp->GetItemName() : bSlotTaken ? FName("Taken") : FName("Empty");
}
AActor* AInventorySlot::GetItem() const {
	return ItemDataComp->GetOwner();
}
FItemGridSize AInventorySlot::GetSize() const {
	if (ItemDataComp) {
		ItemDataComp->GetSize();
	}
	return FItemGridSize();
}

AActor* AInventorySlot::RemoveItem() {
	AActor* retVal = NULL;
	if (ItemDataComp) {
		retVal = ItemDataComp->GetOwner();
		ItemDataComp->GetOwner()->OnDestroyed.RemoveDynamic(this, &AInventorySlot::OnItemSelfDestroy);
		TArray<AInventorySlot*> slots = Inventory->GetSlots(InventoryIndex, ItemDataComp->GetSize(), true);
		for (AInventorySlot* slot : slots) {
			slot->SetSlotEmpty();
		}
	}
	return retVal;
}
bool AInventorySlot::RotateItem()
{
	if (!IsValid(ItemDataComp)) return false;

	int originalWidth = ItemDataComp->GetSize().Width;
	int originalHeight = ItemDataComp->GetSize().Height;
	
	if (originalHeight == originalWidth) {
		ItemDataComp->RotateItem();
		return true;
	}

	int offsetOrigin = InventoryIndex;
	FItemGridSize offsetSize;
	if (originalWidth > originalHeight) {
		offsetOrigin += Inventory->GetInventoryWidth() * originalHeight;
		offsetSize = FItemGridSize(originalHeight, originalWidth - originalHeight);
	}
	else { // originalHeight > originalWidth
		offsetOrigin += originalWidth;
		offsetSize = FItemGridSize(originalHeight - originalWidth, originalWidth);
	}
	TArray<AInventorySlot*> newSlots = Inventory->GetSlots(offsetOrigin, offsetSize, true);
	if(!Inventory->SlotsAreEmpty(newSlots)) {
		return false;
	}
	for (AInventorySlot* slot : newSlots) {
		slot->SetSlotTaken();
	}
	
	offsetSize = offsetSize.GetFlipped();
	if (originalWidth > originalHeight) {
		offsetOrigin = InventoryIndex + originalHeight;
	}
	else {// originalHeight > originalWidth
		offsetOrigin = InventoryIndex + Inventory->GetInventoryWidth() * originalWidth;
	}
	TArray<AInventorySlot*> oldSlots = Inventory->GetSlots(offsetOrigin, offsetSize, true);	
	for (AInventorySlot* slot : oldSlots) {
		slot->SetSlotEmpty();
	}
	ItemDataComp->RotateItem();
	OnSlotUpdated.Broadcast();
	ReplicateSlotUpdated(); 
	return true;
}
void AInventorySlot::DestroyItem()
{
	if (IsValid(ItemDataComp)) {
		AActor* item = ItemDataComp->GetOwner();
		SetItemInfo(NULL);
		item->Destroy(true);		
	}
}

void AInventorySlot::OnInventoryWidthChange(int increaseAmount)
{
	int indexDelta = increaseAmount * (InventoryIndex / Inventory->GetInventoryWidth());
	InventoryIndex += indexDelta;
}

bool AInventorySlot::MatchesItem(AActor* otherItem) const {
	if (IsValid(otherItem) && IsValid(ItemDataComp)) {
		if (UItemDataComponent* otherComp = otherItem->GetComponentByClass<UItemDataComponent>()) {
			if (otherComp->GetItemName() ==ItemDataComp->GetItemName()) {
				return true;
			}
		}
	}
	return false;
}
bool AInventorySlot::IsEmpty() const
{
	return (IsValid(ItemDataComp) && (!bSlotTaken));
}

////////////////////////////////////////////////////////////////////////////////PRIVATE////////////////////////////////////////////////////////////////////////////////
void AInventorySlot::SetSlotTaken() {
	bSlotTaken = true;
	ItemDataComp = NULL;
}
void AInventorySlot::SetSlotEmpty() {
	bSlotTaken = false;
	if (IsValid(ItemDataComp)) {
		ItemProfile.Empty();
	}
	ItemDataComp = NULL;
}

void AInventorySlot::SetSlotTakenStatus_Server_Implementation(bool newTaken) {
	bSlotTaken = newTaken;
}

void AInventorySlot::AttachToInventory_Server_Implementation(UInventoryComponent* inventory, int index)
{
	Inventory = inventory;
	InventoryIndex = index;
}

void AInventorySlot::ReplicateSlotUpdated_Implementation()
{
	OnSlotUpdated.Broadcast();
}

void AInventorySlot::UpdateItem_Server_Implementation(AActor* item, UItemDataComponent* dataComp) {
	RemoveItem();
	if (!IsValid(item) || !IsValid(dataComp)) {
		OnSlotUpdated.Broadcast();
		ReplicateSlotUpdated();
		return;
	}
	TArray<AInventorySlot*> slots = Inventory->GetSlots(InventoryIndex, dataComp->GetSize(), true);
	for (AInventorySlot* slot : slots) {
		if (slot == this) {
			ItemDataComp = dataComp;
			ItemProfile.Empty();
			for (UActorComponent* comp : item->GetComponents()) {
				if (UPrimitiveComponent* prim = Cast<UPrimitiveComponent>(comp)) {
					ItemProfile.Add(prim);
				}
			}
			SetItemPhysicality(false, false);
		}
		else {
			slot->SetSlotTaken();
		}
	}
	OnSlotUpdated.Broadcast();
	ReplicateSlotUpdated();
}

void AInventorySlot::SetItemPhysicality(bool visible, bool bOnlyVisibleInRenderTargets) {
	if (HasAuthority()) {
		ReplicateItemPhysicality(visible, bOnlyVisibleInRenderTargets);
	}
}

void AInventorySlot::ReplicateItemPhysicality_Implementation(bool visible, bool bOnlyVisibleInRenderTargets) {
	for (int i = 0; i < ItemProfile.Num(); i++) {
		if(IsValid(ItemProfile[i].Component)) {
			ItemProfile[i].Component->SetVisibility(visible);
			ItemProfile[i].Component->SetVisibleInSceneCaptureOnly(bOnlyVisibleInRenderTargets);
		}
	}
}

void AInventorySlot::OnItemSelfDestroy(AActor* destroyedActor) {
	SetItemInfo(NULL);
}
	
	*/