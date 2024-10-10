// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ReplicatedDragHolder.h"
#include "Item/ItemDataComponent.h"
#include "InventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "Blueprint/DragDropOperation.h"

// Sets default values
AReplicatedDragHolder::AReplicatedDragHolder()
{
	bReplicates = (true);
	bAlwaysRelevant = true;

}

// Called when the game starts or when spawned
void AReplicatedDragHolder::BeginPlay()
{
	Super::BeginPlay();	
	if(GetOwner()->GetInstigator()->IsLocallyControlled())
	{
		OwningPlayer = Cast<APlayerController>(GetOwner()->GetInstigator()->GetController());
	}
}

void AReplicatedDragHolder::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AReplicatedDragHolder, OriginalInventory);
	DOREPLIFETIME(AReplicatedDragHolder, OriginalIndex);
	DOREPLIFETIME(AReplicatedDragHolder, HoldingItem);
	DOREPLIFETIME(AReplicatedDragHolder, bPreviewRotated);
}

// Called every frame
void AReplicatedDragHolder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AReplicatedDragHolder::RotateHeldItem()
{
	if (!HoldingItem) return;
	if (!HasAuthority())
	{
		RotateReplicatedItem_Server();
	}
	else
	{
		bPreviewRotated = !bPreviewRotated;
		HoldingItem->RotateItem();
	}

}

void AReplicatedDragHolder::HoldItem(UItemDataComponent* item, int index)
{
	if (OwningPlayer)
	{
		EnableInput(OwningPlayer);
	}
	if (HasAuthority())
		UpdateItemInfo(item, index);
	else
		HoldItem_Server(item, index);
}

void AReplicatedDragHolder::ReturnItem()
{
	if (HasAuthority())
	{
		if (!IsValid(OriginalInventory) || OriginalIndex < 0 || !IsValid(HoldingItem)) return;

		if (bPreviewRotated) HoldingItem->RotateItem();

		int addedAt = OriginalInventory->AddItemToInventory(HoldingItem->GetOwner(), OriginalIndex);

		if (addedAt < 0)
		{
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Orange, FString("Unable to return item. Falling back to any index."));
			OriginalInventory->AddItemToInventory(HoldingItem->GetOwner());
		}

		UpdateItemInfo(nullptr, -1);
		Cleanup();//GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AReplicatedDragHolder::Cleanup);
	}
	else
		ReturnItem_Server();
}

UItemDataComponent* AReplicatedDragHolder::RetrieveItem(bool bDestroyHolderOnRetrieve)
{
	UItemDataComponent* retVal = HoldingItem;
	if(!bDestroyHolderOnRetrieve)
	{
		if (HasAuthority())
		{
			UpdateItemInfo(nullptr, -1);
			Cleanup();//GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AReplicatedDragHolder::Cleanup);
		}
		else
			RetrieveItem_Server();
	}
	return retVal;
}

void AReplicatedDragHolder::BindReturnToDragCancel(UDragDropOperation* InOperation)
{
	if (!InOperation) return;

	InOperation->OnDragCancelled.AddDynamic(this, &AReplicatedDragHolder::BoundedReturnItemToInventory);
}

void AReplicatedDragHolder::BoundedReturnItemToInventory(UDragDropOperation* InOperation)
{
	OriginalInventory->ReturnItemFromHolder(this);
}

void AReplicatedDragHolder::UpdateItemInfo(UItemDataComponent* itemData, int index)
{
	HoldingItem = itemData;
	OriginalIndex = index;
	if (!HoldingItem)
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &AReplicatedDragHolder::Cleanup);
}

void AReplicatedDragHolder::HoldItem_Server_Implementation(UItemDataComponent* itemData, int index)
{
	HoldItem(itemData, index);
}

void AReplicatedDragHolder::ReturnItem_Server_Implementation()
{
	ReturnItem();
}

void AReplicatedDragHolder::RetrieveItem_Server_Implementation()
{
	RetrieveItem();
}

void AReplicatedDragHolder::RotateReplicatedItem_Server_Implementation()
{
	RotateHeldItem();
}

void AReplicatedDragHolder::Cleanup()
{
	Destroy();
}

