// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/InventoryItemBase.h"
#include "Net/UnrealNetwork.h"

AInventoryItemBase::AInventoryItemBase()
{
	bReplicates = (true);
	bAlwaysRelevant = true;
}

void AInventoryItemBase::BeginPlay()
{
	OnDepleted.AddDynamic(this, &AInventoryItemBase::OnDeplete);

	DynamicImage = UMaterialInstanceDynamic::Create(Image, this);
	OnInfoChange.Broadcast();
}

void AInventoryItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AInventoryItemBase, Quantity);
	DOREPLIFETIME(AInventoryItemBase, Size);
	DOREPLIFETIME(AInventoryItemBase, bRotated);
}

FText AInventoryItemBase::GetGridText_Implementation() const
{
	return GetGridText_Internal();
}

FText AInventoryItemBase::GetGridText_Internal() const
{
	return FText::FromString(FString::FromInt(GetQuantity()));
}

int AInventoryItemBase::GetQuantity() const
{
	return Quantity;
}

int AInventoryItemBase::GetMaxQuantity() const
{
	return FMath::Clamp(MaxQuantity, 0, MaxQuantity);
}

int AInventoryItemBase::SetQuantity(EItemModify modification, int value)
{
	int retVal = value;
	if (modification == EItemModify::Add && value < 0)
	{
		modification = EItemModify::Subtract;
		value = FMath::Abs(value);
	}
	if (modification == EItemModify::Subtract && value < 0)
	{
		modification = EItemModify::Add;
		value = FMath::Abs(value);
	}

	switch (modification)
	{
	case EItemModify::Add:
		if (MaxQuantity <= 0)
			retVal = 0;
		else
			retVal -= (MaxQuantity - Quantity);
		break;
	case EItemModify::Subtract:
		retVal -= Quantity;
		break;
	case EItemModify::Override:
		retVal = 0;
		break;
	}	
	retVal = FMath::Clamp(retVal, 0, retVal);
	if (HasAuthority())
	{
		switch (modification)
		{
		case EItemModify::Add:
			if (MaxQuantity <= 0)
				Quantity += value;
			else
				Quantity = FMath::Clamp(Quantity + value, 0, MaxQuantity);
			break;
		case EItemModify::Subtract:
			Quantity -= value;
			if (Quantity <= 0)
				ReplicateDepleted();
			break;
		case EItemModify::Override:
			Quantity = value;
			if (MaxQuantity > 0)
				Quantity = FMath::Clamp(Quantity, 0, MaxQuantity);
			ReplicateInfoChange();
			if (Quantity <= 0)
				ReplicateDepleted();
			break;
		}
		ReplicateInfoChange();
	}
	else
	{
		SetQuantity_Server(modification, value);
		return retVal;
	}
	return retVal;
}

void AInventoryItemBase::Rotate()
{
	if (HasAuthority())
	{
		FItemGridSize newSize = FItemGridSize(Size.Height, Size.Width);
		Size = newSize;
		bool oldValue = bRotated;
		bRotated = !oldValue;
		OnRep_bRotated(oldValue);
	}
	else
	{
		Rotate_Server();
	}
}

void AInventoryItemBase::OnDeplete_Implementation()
{

}

void AInventoryItemBase::Rotate_Server_Implementation()
{
	Rotate();
}

void AInventoryItemBase::OnRep_bRotated(bool previousValue)
{
	DynamicImage->SetScalarParameterValue(ImageRotateScalarName, bRotated ? 1.f : 0.f);
	OnRotated.Broadcast();
}



void AInventoryItemBase::SetQuantity_Server_Implementation(EItemModify modification, int value)
{
	if (HasAuthority())
	{
		SetQuantity(modification, value);
	}
}

void AInventoryItemBase::ReplicateInfoChange_Implementation()
{
	OnInfoChange.Broadcast();
}

void AInventoryItemBase::ReplicateDepleted_Implementation()
{
	OnDepleted.Broadcast();
}

void AInventoryItemBase::OnRep_Quantity(int oldVale)
{
	OnInfoChange.Broadcast();
}
