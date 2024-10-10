// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Item/InventorySlot.h"
#include "Item/ItemDataComponent.h"
#include "Item/InventoryItemData.h"
#include "UI/ReplicatedDragHolder.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent() {
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}
void UInventoryComponent::BeginPlay() {
	
	Super::BeginPlay();
	if (HasAuthority()) {
		for (int i = 0; i < InventorySize; i++) {
			FActorSpawnParameters spawnParams;
			spawnParams.Owner = GetOwner();
			AInventorySlot* newSlot = GetWorld()->SpawnActor<AInventorySlot>(AInventorySlot::StaticClass(), spawnParams);
			newSlot->AttachToInventory(this, i);
			OnInventoryWidthChange.AddDynamic(newSlot, &AInventorySlot::OnInventoryWidthChange);
			ItemSlots.SetNum(InventorySize);
		}
		OnInventoryGenerated.Broadcast(this);
		ReplicateFinishedGeneratingInventory_Multi();

		FActorSpawnParameters spawnParams;
		spawnParams.Owner = GetOwner();
		NewDragHolder = GetWorld()->SpawnActor<AReplicatedDragHolder>(AReplicatedDragHolder::StaticClass(), spawnParams);
		NewDragHolder->OriginalInventory = this;
	}
	else {
		RequestFinishGeneratingInventory_Server();
	}
	
}
void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, ItemSlots);
	DOREPLIFETIME(UInventoryComponent, InventorySize); 
	DOREPLIFETIME(UInventoryComponent, InventoryWidth);
	DOREPLIFETIME(UInventoryComponent, NewDragHolder);
	DOREPLIFETIME(UInventoryComponent, TakenSlots);

}
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}
TArray<int> UInventoryComponent::GetSlots(int index, FItemGridSize size, bool bIncludeOrigin) const {
	TArray<int> retVal;
	if (size.Width <= 0 || size.Height <= 0) return retVal;
	if (size.Width == 1 && size.Height == 1 && !bIncludeOrigin) return retVal;

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
bool UInventoryComponent::SlotsAreEmpty(TArray<AInventorySlot*> slots) const {

	if (slots.Num() <= 0) return false;
	for (AInventorySlot* s : slots)
	{
		if (!s->IsEmpty()) return false;
	}
	return true;
}
bool UInventoryComponent::SlotsAreEmptyAtIndex(TArray<int> indexes) const {
	if (indexes.Num() <= 0) return false;
	for (int i : indexes)
	{
		if (i > InventorySize || i < 0) return false;

		if (!ItemSlots[i]) return false;
	}
	return true;
}
bool UInventoryComponent::SlotIsEmpty(AInventorySlot* slot) const {
	return slot->IsEmpty();
}
bool UInventoryComponent::SlotIsEmptyAtIndex(int index) const {
	if (index > InventorySize || index < 0) {
		return false;
	}

	return IsValid(ItemSlots[index]) || TakenSlots.Contains(index);
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
				FActorSpawnParameters spawnParams;
				spawnParams.Owner = GetOwner();
				AInventorySlot* newSlot = GetWorld()->SpawnActor<AInventorySlot>(AInventorySlot::StaticClass(), spawnParams);
				newSlot->AttachToInventory(this, (InventoryWidth * i) - 1 + j);
				OnInventoryWidthChange.AddDynamic(newSlot, &AInventorySlot::OnInventoryWidthChange);

				ItemSlots.Insert(newSlot, (InventoryWidth * i) - 1 + j);
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
	FItemGridSize itemSize = itemData->GetSize();
	int itemQuantity = itemData->GetQuantity();

	if (desiredIndex >= 0) {
		if(desiredIndex < InventorySize) {
			TArray<AInventorySlot*> slots = GetSlots(desiredIndex, itemSize, true);
			bool empty = SlotsAreEmpty(slots);
			if (empty) {
				ItemSlots[desiredIndex]->SetItemInfo(item);
				return desiredIndex;
			}
			if (ItemSlots[desiredIndex]->MatchesItem(item))	{
				int originalValue = itemQuantity;
				
				int excessValue = ItemSlots[desiredIndex]->GetItemInfo()->AddQuantity(originalValue);
				itemData->SetQuantity(excessValue);
				if (excessValue < originalValue) {
					return desiredIndex;
				}
				else {
					return -1;
				}
			}
		}
	}
	else {
		int lastIndex = -1;
		int freeIndex = -1;
		for (AInventorySlot* s : ItemSlots)	{
			// Check for a free slot while iterating looking for similar existingitems to iterate.
			if (SlotsAreEmpty(GetSlots(s->GetIndex(), itemSize, true)) && freeIndex < 0)
				freeIndex = s->GetIndex();

			// Found a similar existing item to iterate.
			if (IsValid(item) && s->MatchesItem(item) && itemQuantity > 0) {
				lastIndex = s->GetIndex();
				item->SetQuantity(s->GetItemAsOldBase()->SetQuantity(EItemModify::Add, item->GetQuantity()));
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
			ItemSlots[freeIndex]->SetItemInfo(item);
		}
		// This will be -1 (Not found) if we don't have a free slot or it will be the added slot.
		return freeIndex;
	}
	// Unable to add
	return -1;
}

int UInventoryComponent::AddItemToInventoryUsingData(const FItemDataAmount& item, FItemDataAmount& modifiedItemData, int desiredIndex)
{
	modifiedItemData = item;
	if (!IsValid(item.DataAsset) || item.Quantity == 0) return -1;
	if (desiredIndex > InventorySize) return -1;

	AActor* defaultObj = item.DataAsset->ItemClass->GetDefaultObject<AActor>();
	if (!defaultObj) {
		UE_LOG(LogTemp, Warning, TEXT("The data asset %s does not have an Item Class assigned. It was not added to the inventory."), 
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
		if (ItemSlots[desiredIndex]->IsTaken()) {
			return -1;
		}
		if (IsValid(ItemSlots[desiredIndex]->GetItemInfo())) {
			if (ItemSlots[desiredIndex]->GetItemInfo()->GetQuantityMaxAddend() <= amountToAdd) {
				amountToAdd = ItemSlots[desiredIndex]->GetItemInfo()->AddQuantity(amountToAdd);
				modifiedItemData.Quantity = amountToAdd;
				return desiredIndex;
			}
		}
		else {
			if (HasAuthority()) {
				if (AActor* newItem = GenerateItemWithData(item.DataAsset, amountToAdd)) {
					ItemSlots[desiredIndex]->SetItemInfo(newItem);
				}
			}
			else {
				AddItemWithData_Server(item.DataAsset, amountToAdd, desiredIndex);
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
		// Pretty intensive, aint it?
		TArray<AInventorySlot*> availableSlots;
		TArray<AInventorySlot*> blacklistedSlots;
		TArray<AInventorySlot*> existingSlots;
		TArray<AInventorySlot*> currentSlots;
		for (AInventorySlot* s : ItemSlots) {
			if (amountToAdd > 0) {
				if (s->GetItemInfo() && s->GetItemInfo()->MatchesItem(item.DataAsset->Name)) {
					if (s->GetItemInfo()->GetQuantityMaxAddend() > 0) {
						existingSlots.Add(s);
						amountToAdd -= s->GetItemInfo()->GetQuantityMaxAddend();
					}
				}
				// Check if existing slots hasn't already exhausted leftToAdd, we already have enough available slots to exhaust the quantity, the current slot hasn't been used by a previous available one, and that the current slot can fit the item.
				currentSlots = GetSlots(s->GetIndex(), newItemSize, true);
				bool slotIsAvailable = SlotsAreEmpty(currentSlots);				
				if ((amountToAdd >= (availableSlots.Num() * itemMaxQuantity)) && !blacklistedSlots.Contains(s) && slotIsAvailable) {
					availableSlots.Add(s);
					for (AInventorySlot* eS : currentSlots) {
						blacklistedSlots.Add(eS);
					}
				}
			}
		}
		if (amountToAdd <= (availableSlots.Num() * itemMaxQuantity)) {
			amountToAdd = item.Quantity;
			for (AInventorySlot* s : existingSlots) {
				// TODO FIX ME
				// amountToAdd = s->GetItemAsOldBase()->SetQuantity(EItemModify::Add, amountToAdd);
				if (amountToAdd <= 0) {
					modifiedItemData.Quantity = FMath::Max(amountToAdd, 0);
					return s->GetIndex();
				}
			}
			for (AInventorySlot* s : availableSlots) {
				if (HasAuthority()) {
					if (AActor* newItem = GenerateItemWithData(item.DataAsset, amountToAdd)) {
						ItemSlots[s->GetIndex()]->SetItemInfo(newItem);
					}
				}
				else
					AddItemWithData_Server(item.DataAsset, amountToAdd, s->GetIndex());
				amountToAdd -= itemMaxQuantity;
				if (amountToAdd <= 0) {
					modifiedItemData.Quantity = FMath::Max(amountToAdd, 0);
					return s->GetIndex();
				}
			}
		}
	}
	return -1;
}/**/
void UInventoryComponent::DestroyItem(int index) {
	if (index >= InventorySize || index < 0) return;

	ItemSlots[index]->DestroyItem();
}
AActor* UInventoryComponent::RemoveItem(int index) {
	if (index < 0 || index >= InventorySize) {
		return nullptr;
	}

	if (AInventorySlot* s = ItemSlots[index]) {
		if (!IsValid(s->GetItem())) {
			return nullptr;
		}
	}
	else {
		return nullptr;
	}

	AActor* retVal = ItemSlots[index]->RemoveItem();
	return retVal;

}
AReplicatedDragHolder* UInventoryComponent::GetItemHolder(UItemDataComponent* holdItem, int holdItemIndex) {
	AReplicatedDragHolder* oldHolder = NewDragHolder;
	if (HasAuthority()) {
		NewDragHolder->HoldItem(holdItem, holdItemIndex);
		FActorSpawnParameters spawnParams;
		spawnParams.Owner = GetOwner();
		NewDragHolder = GetWorld()->SpawnActor<AReplicatedDragHolder>(AReplicatedDragHolder::StaticClass(), spawnParams);
		NewDragHolder->OriginalInventory = this;
	}
	else
		GenerateItemHolder_Server(holdItem, holdItemIndex);
	return oldHolder;
}
void UInventoryComponent::ReturnItemFromHolder(AReplicatedDragHolder* holder) {
	if (!holder) return;
	if (HasAuthority()) {
		holder->ReturnItem();
		holder->Destroy();
	}
	else
		ReturnItemFromHolder_Server(holder);
}
bool UInventoryComponent::HasAuthority() const {
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
		ItemSlots[desiredIndex] = newItem-;
	}
}
AActor* UInventoryComponent::GenerateItemWithData(UInventoryItemData* itemData, int quantity) {
	AActor* newItem = GetWorld()->SpawnActorDeferred<AActor>(itemData->ItemClass, FTransform(), GetOwner(), GetOwner()->GetInstigator());
	if (IsValid(newItem)) {
		UItemDataComponent* itemComp = Cast<UItemDataComponent>(newItem->AddComponentByClass(UItemDataComponent::StaticClass(), false, FTransform(), true));
		if (IsValid(itemComp)) {
			itemComp->ItemDataAsset = itemData;
			newItem->FinishAddComponent(itemComp, true, FTransform());
			itemComp->RegisterComponent();
			itemComp->SetQuantity(quantity);
			newItem->FinishSpawning(FTransform());
			return newItem;
		}
		itemComp->DestroyComponent();
	}
	newItem->Destroy();
	return NULL;
}
void UInventoryComponent::IncreaseInventorySize_Server_Implementation(int amount) {
	ItemSlots.SetNum(ItemSlots.Num() + amount);
}
void UInventoryComponent::ReplicateFinishedGeneratingInventory_Multi_Implementation() {
	OnInventoryGenerated.Broadcast(this);
	OnFinishGeneratingInventory(this);
}
void UInventoryComponent::RequestFinishGeneratingInventory_Server_Implementation() {
	GetWorld()->GetTimerManager().SetTimerForNextTick<UInventoryComponent>(this, &UInventoryComponent::ReplicateFinishedGeneratingInventory_Multi);
}
void UInventoryComponent::GenerateItemHolder_Server_Implementation(UItemDataComponent* holdItem, int holdItemIndex) {
	if (!HasAuthority()) return;
	NewDragHolder->HoldItem(holdItem, holdItemIndex);
	FActorSpawnParameters spawnParams;
	spawnParams.Owner = GetOwner();
	NewDragHolder = GetWorld()->SpawnActor<AReplicatedDragHolder>(AReplicatedDragHolder::StaticClass(), spawnParams);
	NewDragHolder->OriginalInventory = this;
}
void UInventoryComponent::ReturnItemFromHolder_Server_Implementation(AReplicatedDragHolder* holder) {
	if (!holder) return;
	holder->ReturnItem();
	holder->Destroy();
}


