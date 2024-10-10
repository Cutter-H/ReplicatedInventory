// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemActorBase.h"
#include "Item/InventoryItemBase.h"
#include "Item/InventoryItemData.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AItemActorBase::AItemActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TEXT"));
	SetRootComponent(Mesh);

}

// Called when the game starts or when spawned
void AItemActorBase::BeginPlay()
{
	Super::BeginPlay();
	if(HasAuthority())
	{
		if (SpawnFrom == EItemActorSpawnChoice::DataAsset)
		{
			if (!DataAsset)
				Destroy();
			SetItemByDataAsset(DataAsset, QuantityOverride);
		}
		else
		{
			if (!ItemClass)
				Destroy();
			SetItemByClass(ItemClass, QuantityOverride);
		}
	}
}

void AItemActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemActorBase, Item);
}

// Called every frame
void AItemActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AInventoryItemBase* AItemActorBase::SetItemInfo(AInventoryItemBase* newItem)
{
	AInventoryItemBase* item = nullptr;
	if (Item)
		item = Item;
	AssignItem(newItem);
	return item;
}

AInventoryItemBase* AItemActorBase::SetItemByDataAsset(UInventoryItemData* newItemDataAsset, int quantityOverride)
{
	AInventoryItemBase* item = nullptr;
	if (Item)
		item = Item;
	int quantity;// = newItemDataAsset->Quantity;
	if (quantityOverride > 0)
		quantity = quantityOverride;
	//CreateItemAndAssign(newItemDataAsset->ItemClass, quantity);
	return item;
}

AInventoryItemBase* AItemActorBase::SetItemByClass(TSubclassOf<AInventoryItemBase> newItemClass, int quantityOverride)
{
	AInventoryItemBase* item = nullptr;
	if (Item)
		item = Item;
	int quantity = 1;
	if (quantityOverride > 0)
		quantity = quantityOverride;
	//CreateItemAndAssign(newItemClass, quantity);
	return item;
}

void AItemActorBase::CreateItemAndAssign_Implementation(TSubclassOf<AInventoryItemBase> newItemClass, int itemQuantity)
{
	if (!IsValid(newItemClass) || itemQuantity <= 0 || !HasAuthority()) return;

	AInventoryItemBase* newItem = GetWorld()->SpawnActorDeferred<AInventoryItemBase>(newItemClass, FTransform(), this);
	newItem->SetQuantity(EItemModify::Override, itemQuantity);
	newItem->FinishSpawning(FTransform());
	Item = newItem;
	UpdateMesh();
}

void AItemActorBase::AssignItem_Implementation(AInventoryItemBase* newItem)
{
	Item = newItem;
	UpdateMesh();
}

void AItemActorBase::UpdateMesh_Implementation()
{
	if (!Item) return;
	if (UStaticMeshComponent* mesh = Item->ItemPayload->GetComponentByClass<UStaticMeshComponent>()) {
		Mesh->SetStaticMesh(mesh->GetStaticMesh());
	}
}

