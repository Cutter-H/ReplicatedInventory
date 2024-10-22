// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "ReplicatedInventory.h"
#include "Item/ItemDataComponent.h"
#include "Item/InventoryItemData.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent() {
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}
void UInventoryComponent::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) {
		// Resize item slot array.
		ItemSlots.SetNum(InventorySize, false);

		OnInventoryGenerated.Broadcast(this);
		ReplicateFinishedGeneratingInventory_Multi();
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = GetOwner();
		ReplicateFinishedGeneratingInventory_Multi();
	}
	
}
void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UInventoryComponent, ItemSlots, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(UInventoryComponent, InventorySize); 
	DOREPLIFETIME(UInventoryComponent, InventoryWidth);
	DOREPLIFETIME(UInventoryComponent, TakenSlots);

}
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}
TArray<int> UInventoryComponent::GetSlots(int index, FItemGridSize size, bool bIncludeOrigin) const {
	TArray<int> retVal;
	if (size.Width <= 0 || size.Height <= 0) {
		UE_LOG(LogInventory, Warning, TEXT("Attempted to get slots with an invalid size."))
		return retVal;
	}
	if (size.Width == 1 && size.Height == 1 && !bIncludeOrigin) {
		UE_LOG(LogInventory, Warning, TEXT("Attempted to get slots with a size of 1 while excluding origin."))
		return retVal;
	}

	int column = index % InventoryWidth;
	int lastSlot = index + (size.Width - 1) + ((size.Height - 1) * InventoryWidth);

	if (column + (size.Width - 1) >= InventoryWidth) return retVal;
	if (lastSlot >= InventorySize) return retVal;


	if (bIncludeOrigin)
		retVal.Add(index);
	
	int currentIndex = -1;
	for (int w = 0; w < size.Width; w++)
	{
		for (int h = 0; h < size.Height; h++)
		{
			currentIndex = index + w + (InventoryWidth * h);
			if(currentIndex != index)
				retVal.Add(currentIndex);
		}
	}
	return retVal;
}
bool UInventoryComponent::SlotsAreEmpty(TArray<int> indexes) const {
	if (indexes.Num() <= 0) {
		return false;
	}
	for (int i : indexes) {
		if (!SlotIsEmpty(i)) {
			return false;
		}
	}
	return true;
}
bool UInventoryComponent::SlotIsEmpty(int index) const {
	if (!IsValidIndex(index) || TakenSlots.Contains(index)) {
		return false;
	}

	return !IsValid(ItemSlots[index]);
}
int UInventoryComponent::Index2DToInt(FInventory2DIndex index2D) const {
	return index2D.X + (index2D.Y * InventoryWidth);
}
FInventory2DIndex UInventoryComponent::IndexIntTo2D(int index) const {
	return FInventory2DIndex(index, InventoryWidth);
}
void UInventoryComponent::IncreaseInventorySize(int increaseAmount) {
	if (increaseAmount <= 0) return;
	if (HasAuthority()) {
		ItemSlots.SetNum(ItemSlots.Num() + increaseAmount);
	}
	else
		IncreaseInventorySize_Server(increaseAmount);
}
bool UInventoryComponent::IncreaseInventoryWidth(int increaseAmount) {
	if (InventorySize < InventoryWidth)	return false;
	if (HasAuthority()) {
		OnInventoryWidthChange.Broadcast(increaseAmount);
		for (int i = 1; i <= InventorySize / InventoryWidth; i++) {
			for (int j = 0; j < increaseAmount; j++) {
				ItemSlots.Insert(NULL, (InventoryWidth * i) - 1 + j);
			}
		}
		OnInventoryChange.Broadcast();
	}
	else
		IncreaseInventoryWidth_Server(increaseAmount);
	return true;
}
int UInventoryComponent::AddItemToInventory(AActor* item, int desiredIndex) {
	if (!IsValid(item)) {
		return -1;
	}
	UItemDataComponent* itemData = item->FindComponentByClass<UItemDataComponent>();
	if (!IsValid(itemData)) {
		return -1;
	}
	if (ItemSlots.Contains(itemData)) { // Avoid duplicates.
		return -1;
	}
	FItemGridSize itemSize = itemData->GetSize();
	int itemQuantity = itemData->GetQuantity();

	if (desiredIndex >= 0) {
		if(desiredIndex < InventorySize) {
			TArray<int> slots = GetSlots(desiredIndex, itemSize, true);
			bool empty = true;
			if (!itemSize.IsSingle()) {
				empty = SlotsAreEmpty(slots);
			}
			if (empty) {
				SetItemOnServer(desiredIndex, itemData);
				return desiredIndex;
			}
			if (IsValid(ItemSlots[desiredIndex])) {
				if (ItemSlots[desiredIndex]->MatchesItem(itemData->GetItemName())) {
					int originalValue = itemQuantity;

					int excessValue = ItemSlots[desiredIndex]->AddQuantity(originalValue);
					itemData->SetQuantity(excessValue);
					if (excessValue < originalValue) {
						return desiredIndex;
					}
				}
			}
		}
	}
	else {
		int lastIndex = -1;
		int freeIndex = -1;
		for (int i = 0; i < ItemSlots.Num(); i++) {
			UItemDataComponent* s = ItemSlots[i];
			// Check for a free slot while iterating looking for similar existingitems to iterate.
			if (SlotsAreEmpty(GetSlots(i, itemSize, true)) && freeIndex < 0)
				freeIndex = i;

			// Found a similar existing item to iterate.
			if (IsValid(item) && IsValid(s) && s->MatchesItem(itemData->GetItemName()) && itemQuantity > 0) {
				lastIndex = i;
				itemData->SetQuantity(s->AddQuantity(itemData->GetQuantity()));
			}

			// Exit the loop if the we found a empty slot or ran out iterating existing items. No reason to finish the array if we're done.
			if (freeIndex >= 0 && (IsValid(item) || itemQuantity <= 0)) {
				break;
			}
		}
		// We have exhausted the added item on existing items.
		if (!IsValid(item) || itemQuantity <= 0) {
			return lastIndex;
		}
		// We found a free slot and still have quantity of the item to add, so we add it to the freeIndex found earlier.
		if (freeIndex >= 0) {
			SetItemOnServer(freeIndex, itemData);
		}
		// This will be -1 (Not found) if we don't have a free slot or it will be the added slot.
		return freeIndex;
	}
	// Unable to add
	return -1;
}
int UInventoryComponent::AddItemComponentToInventory(UItemDataComponent* itemData, int desiredIndex) {
	if (IsValid(itemData)) {
		return AddItemToInventory(itemData->GetOwner(), desiredIndex);
	}
	return -1;
}
int UInventoryComponent::AddItemToInventoryUsingData(const FItemDataAmount& item, FItemDataAmount& modifiedItemData, int desiredIndex)
{
	modifiedItemData = item;
	if (!IsValid(item.DataAsset)) {
		UE_LOG(LogInventory, Warning, TEXT("The item you attempted to add does not have a valid Data Asset."));
		return -1;
	}
	if (item.Quantity <= 0) {
		UE_LOG(LogInventory, Warning, TEXT("The item you attempted to add 0 or less quantity."));
	}
	if (desiredIndex > InventorySize) {
		UE_LOG(LogInventory, Warning, TEXT("The index you attempted to add to is greater than the inventory's size."));
	}
	AActor* defaultObj;
	if (IsValid(item.DataAsset->ItemClass)) {
		defaultObj = item.DataAsset->ItemClass->GetDefaultObject<AActor>();
	} 
	else {
		UE_LOG(LogInventory, Warning, TEXT("The data asset %s does not have an Item Class assigned. It was not added to the inventory."), 
			*GetNameSafe(item.DataAsset));
		return -1;
	}
	// Why is this here?
	//defaultObj->SetLifeSpan(1.f);

	FItemGridSize newItemSize = item.DataAsset->Size;
	int amountToAdd = item.Quantity;
	int itemMaxQuantity = item.DataAsset->MaxQuantity;

	// Adding to specific slot
	if (desiredIndex >= 0) {
		if (TakenSlots.Contains(desiredIndex)) {
			UE_LOG(LogInventory, Warning, TEXT("The index you attempted to add the item to is a taken slot."));
			return -1;
		}
		if (IsValid(ItemSlots[desiredIndex])) {
			if (ItemSlots[desiredIndex]->GetQuantityMaxAddend() <= amountToAdd) {
				amountToAdd = ItemSlots[desiredIndex]->AddQuantity(amountToAdd);
				modifiedItemData.Quantity = amountToAdd;
				return desiredIndex;
			}
		}
		else {
			if (HasAuthority()) {
				if (AActor* newItem = GenerateItemWithData(item.DataAsset, amountToAdd)) {
					if (UItemDataComponent* newItemData = newItem->FindComponentByClass<UItemDataComponent>()) {
						SetItemOnServer(desiredIndex, newItemData);
					}
					else {
						newItem->Destroy();
					}
				}
			}
			else {
				AddItemToIndexWithData_Server(item.DataAsset, amountToAdd, desiredIndex);
			}
			if (itemMaxQuantity < amountToAdd) {
				modifiedItemData.Quantity -= itemMaxQuantity;
			}
			else {
				modifiedItemData.Quantity = 0;
			}
			return desiredIndex;
			
		}
	}
	else { // No specific slot identified. Add until exhausted.
		TArray<int> availableSlots;
		TArray<int> blacklistedSlots;
		TArray<int> existingSlots;
		TArray<int> currentSlots;
		for (int i = 0; i < InventorySize; i++) {
			UItemDataComponent* s = ItemSlots[i];
			if (!blacklistedSlots.Contains(i)) {
				if (amountToAdd > 0) {
					if (IsValid(s) && s->MatchesItem(item.DataAsset->Name)) {
						if (s->GetQuantityMaxAddend() > 0) {
							existingSlots.Add(i);
							amountToAdd -= s->GetQuantityMaxAddend();
						}
					}
					else {
						if (SlotIsEmpty(i)) {
							if (!newItemSize.IsSingle()) {
								currentSlots = GetSlots(i, newItemSize, false);
								bool slotIsAvailable = SlotsAreEmpty(currentSlots);
								if (slotIsAvailable) {
									availableSlots.Add(i);
									blacklistedSlots.Append(currentSlots);
								}
							}
							else {
								availableSlots.Add(i);
							}
						}
					}
				}
			}
			
		}
		if (amountToAdd <= (availableSlots.Num() * itemMaxQuantity)) {
			amountToAdd = item.Quantity;
			for (int s : existingSlots) {
				
				amountToAdd = ItemSlots[s]->AddQuantity(amountToAdd);
				if (amountToAdd <= 0) {
					modifiedItemData.Quantity = FMath::Max(amountToAdd, 0);
					UE_LOG(LogInventory, Log, TEXT("Added %s to index %s."), *item.DataAsset->Name.ToString(), *FString::FromInt(s));
					return s;
				}
			}
			for (int s : availableSlots) {
				if (HasAuthority()) {
					AActor* newItem = GenerateItemWithData(item.DataAsset, amountToAdd);
					if (IsValid(newItem)) {
						if (UItemDataComponent* newItemData = newItem->FindComponentByClass<UItemDataComponent>()) {
							SetItemOnServer(s, newItemData);
						}
						else {
							newItem->Destroy();
						}
					}
				}
				else
					AddItemToIndexWithData_Server(item.DataAsset, amountToAdd, s);
				amountToAdd -= itemMaxQuantity;
				if (amountToAdd <= 0) {
					modifiedItemData.Quantity = FMath::Max(amountToAdd, 0);
					UE_LOG(LogInventory, Log, TEXT("Added %s to index %s."), *item.DataAsset->Name.ToString(), *FString::FromInt(s));
					return s;
				}
			}
		}
	}
	UE_LOG(LogInventory, Warning, TEXT("The item could not be added to the inventory."));
	return -1;
}/**/
void UInventoryComponent::DestroyItem(int index) {
	if (!ItemSlots.IsValidIndex(index)) {
		return;
	}
	AActor* item = RemoveItem(index);
	if (!IsValid(item)) {
		return;
	}
	item->Destroy(true);
	
}
AActor* UInventoryComponent::RemoveItem(int index) {
	if (!ItemSlots.IsValidIndex(index)) {
		return nullptr;
	}
	UItemDataComponent* item = ItemSlots[index];
	if (!IsValid(item)) {
		return nullptr;
	}
	TArray<int> takenSlots = GetSlots(index, item->GetSize());
	for (int s : takenSlots) {
		if (HasAuthority()) {
			TakenSlots.Remove(s);
			ReplicateTakenSlotChange_Multi(s, false);
		}
		else {
			UpdateTakenSlotChange_Server(s, false);
		}
	}
	SetItemOnServer(index, NULL);
	return item->GetOwner();

}
bool UInventoryComponent::CanRotateSlot(int index) const {
	if (!IsValidIndex(index)) {
		return false;
	}
	UItemDataComponent* item = ItemSlots[index];
	if (!IsValid(item)) {
		return false;
	}
	FItemGridSize originalSize = item->GetSize();
	if (originalSize.Height == originalSize.Width) {
		return true;
	}

	int offsetOrigin = index;
	FItemGridSize offsetSize;
	if (originalSize.Width > originalSize.Height) {
		offsetOrigin += GetInventoryWidth() * originalSize.Height;
		offsetSize = FItemGridSize(originalSize.Height, originalSize.Width - originalSize.Height);
	}
	else { // originalHeight > originalWidth
		offsetOrigin += originalSize.Width;
		offsetSize = FItemGridSize(originalSize.Height - originalSize.Width, originalSize.Width);
	}
	TArray<int> newSlots = GetSlots(offsetOrigin, offsetSize, true);
	if (!SlotsAreEmpty(newSlots)) {
		return false;
	}
	return true;
}
bool UInventoryComponent::RotateSlot(int index) {
	if (!CanRotateSlot(index)) {
		return false;
	}
	if (!HasAuthority()) {
		RotateItemOnServer(index);
		return true;
	}
	FItemGridSize oldSize = ItemSlots[index]->GetSize();
	ItemSlots[index]->RotateItem();
	if (!oldSize.IsSingle()) {
		TArray<int> slots = GetSlots(index, oldSize);
		for (int i : slots) {
			TakenSlots.Remove(i);
			ReplicateTakenSlotChange_Multi(i, false);
		}
		slots = GetSlots(index, oldSize.GetFlipped());
		for (int i : slots) {
			TakenSlots.Add(i);
			ReplicateTakenSlotChange_Multi(i, true);
		}
	}
	OnInventorySlotRotated.Broadcast(index, oldSize.GetFlipped());
	ReplicateSlotRotated_Multi(index, oldSize.GetFlipped());
	return true;
}


void UInventoryComponent::SetItemOnServer_Implementation(int index, UItemDataComponent* item) {
	if (IsValid(item)) {
		FItemGridSize size = item->GetSize();
		if (!size.IsSingle()) {
			TArray<int> itemSlots = GetSlots(index, size);
			if (itemSlots.Num() <= 0) {
				return;
			}
			for (int i : itemSlots) {
				TakenSlots.Add(i);
				ReplicateTakenSlotChange_Multi(i, true);
			}
		}
		
		item->GetOwner()->SetOwner(GetOwner());
		ItemSlots[index] = item;
		OnInventorySlotChange.Broadcast(index, item, EInventorySlotState::Used); 
		UpdatedIndex_Multi(index, item, EInventorySlotState::Used);
	}
	else {
		UItemDataComponent* oldItemData = ItemSlots[index];
		if (IsValid(oldItemData)) {
			TArray<int> itemSlots = GetSlots(index, oldItemData->GetSize());
			for (int i : itemSlots) {
				TakenSlots.Remove(i);
				ReplicateTakenSlotChange_Multi(i, false);
			}
			ItemSlots[index] = nullptr;
			OnInventorySlotChange.Broadcast(index, nullptr, EInventorySlotState::Empty);
			UpdatedIndex_Multi(index, nullptr, EInventorySlotState::Empty);
		}
	}
}
void UInventoryComponent::SetItem(int index, UItemDataComponent* item) {
	if (!HasAuthority()) {
		return;
	}
	if (IsValid(item)) {
		FItemGridSize size = item->GetSize();
		if (!size.IsSingle()) {
			TArray<int> itemSlots = GetSlots(index, size);
			if (itemSlots.Num() <= 0) {
				return;
			}
			for (int i : itemSlots) {
				TakenSlots.Add(i);
				ReplicateTakenSlotChange_Multi(i, true);
			}
		}

		item->GetOwner()->SetOwner(GetOwner());
		ItemSlots[index] = item;
		OnInventorySlotChange.Broadcast(index, item, EInventorySlotState::Used);
		UpdatedIndex_Multi(index, item, EInventorySlotState::Used);
	}
	else {
		UItemDataComponent* oldItemData = ItemSlots[index];
		if (IsValid(oldItemData)) {
			TArray<int> itemSlots = GetSlots(index, oldItemData->GetSize());
			for (int i : itemSlots) {
				TakenSlots.Remove(i);
				ReplicateTakenSlotChange_Multi(i, false);
			}
			ItemSlots[index] = nullptr;
			OnInventorySlotChange.Broadcast(index, nullptr, EInventorySlotState::Empty);
			UpdatedIndex_Multi(index, nullptr, EInventorySlotState::Empty);
		}
	}
}
bool UInventoryComponent::HasAuthority() const {
	if (!GetOwner()->GetIsReplicated()) {
		return true;
	}
	return (IsValid(GetOwner()) && GetOwner()->HasAuthority());
}
bool UInventoryComponent::IsLocallyControlled() const {
	if (APawn* owningPawn = Cast<APawn>(GetOwner())) {
		return owningPawn->IsLocallyControlled();
	}
	return false;
}
void UInventoryComponent::IncreaseInventoryWidth_Server_Implementation(int amount) {
	IncreaseInventoryWidth(amount);
}
void UInventoryComponent::AddItemToIndexWithData_Server_Implementation(UInventoryItemData* itemData, int quantity, int desiredIndex) {
	if (AActor* newItem = GenerateItemWithData(itemData, quantity)) {
		if (UItemDataComponent* itemComp = newItem->FindComponentByClass<UItemDataComponent>()) {
			SetItem(desiredIndex, itemComp);
		}
		else {
			newItem->Destroy();
		}
	}
}
AActor* UInventoryComponent::GenerateItemWithData(UInventoryItemData* itemData, int quantity) {
	if (!IsValid(itemData)) {
		return nullptr;
	}
	AActor* newItem = GetWorld()->SpawnActorDeferred<AActor>(itemData->ItemClass, FTransform(), GetOwner(), GetOwner()->GetInstigator(), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (IsValid(newItem)) {
		newItem->SetReplicates(true);
		UItemDataComponent* itemComp = Cast<UItemDataComponent>(newItem->AddComponentByClass(UItemDataComponent::StaticClass(), false, FTransform(), true));
		if (IsValid(itemComp)) {
			itemComp->ItemDataAsset = itemData;
			newItem->FinishAddComponent(itemComp, true, FTransform());
			
			itemComp->RegisterComponent();
			newItem->FinishSpawning(FTransform());

			itemComp->ReplicateDataAssetInfo(itemData);
			itemComp->SetQuantity(quantity);
			UE_LOG(LogInventory, Log, TEXT("Spawned the item %s"), *itemData->Name.ToString());
			return newItem;
		}
		else {
			itemComp->DestroyComponent(); 
			newItem->Destroy();
		}
		return nullptr;
	}
	newItem->Destroy();
	return nullptr;
}
void UInventoryComponent::UpdatedIndex_Multi_Implementation(int index, UItemDataComponent* item, EInventorySlotState newSlotState) {
	if (!HasAuthority()) {
		OnInventorySlotChange.Broadcast(index, item, newSlotState);
		
	}
}
void UInventoryComponent::OnRep_ItemSlots(TArray<UItemDataComponent*> oldItemSlots) {
	int max = ItemSlots.Num();
	int modifiedIndex = -1;
	EInventorySlotState state;
	for (int i = 0; i < max; i++) {
		if (oldItemSlots.Num() >= i) {
			modifiedIndex = i;
			break;
		}
		if (oldItemSlots[i] != ItemSlots[i]) {
			modifiedIndex = i;
			break;
		}
	}
	if (!IsValidIndex(modifiedIndex)) {
		return;
	}
	if (!IsValid(ItemSlots[modifiedIndex])) {
		if (TakenSlots.Contains(modifiedIndex)) {
			state = EInventorySlotState::Taken;
		}
		else {
			state = EInventorySlotState::Empty;
		}
	}
	else {
		state = EInventorySlotState::Used;
	}

	OnInventorySlotChange.Broadcast(modifiedIndex, ItemSlots[modifiedIndex], state);
}
void UInventoryComponent::ReplicateTakenSlotChange_Multi_Implementation(int index, bool taken) {
	OnInventorySlotChange.Broadcast(index, nullptr, taken ? EInventorySlotState::Taken : EInventorySlotState::Empty);
}
void UInventoryComponent::UpdateTakenSlotChange_Server_Implementation(int index, bool taken) {
	if (taken) {
		TakenSlots.Add(index);
	}
	else{
		TakenSlots.Remove(index);
	}
	ReplicateTakenSlotChange_Multi(index, taken);
}
void UInventoryComponent::IncreaseInventorySize_Server_Implementation(int amount) {
	ItemSlots.SetNum(ItemSlots.Num() + amount);
}
void UInventoryComponent::ReplicateFinishedGeneratingInventory_Multi_Implementation() {
	OnInventoryGenerated.Broadcast(this);
	OnFinishGeneratingInventory(this);
}
void UInventoryComponent::RequestFinishGeneratingInventory_Server_Implementation() {

}
void UInventoryComponent::RotateItemOnServer_Implementation(int index) {
	FItemGridSize oldSize = ItemSlots[index]->GetSize();
	ItemSlots[index]->RotateItem();
	if (!oldSize.IsSingle()) {
		TArray<int> slots = GetSlots(index, oldSize);
		for (int i : slots) {
			TakenSlots.Remove(i);
			ReplicateTakenSlotChange_Multi(i, false);
		}
		slots = GetSlots(index, oldSize.GetFlipped());
		for (int i : slots) {
			TakenSlots.Add(i);
			ReplicateTakenSlotChange_Multi(i, true);
		}
	}
	OnInventorySlotRotated.Broadcast(index, oldSize.GetFlipped());
	ReplicateSlotRotated_Multi(index, oldSize.GetFlipped());
}

void UInventoryComponent::ReplicateSlotRotated_Multi_Implementation(int index, FItemGridSize newSize) {
	if (!HasAuthority()) {
		OnInventorySlotRotated.Broadcast(index, newSize);
	}
}