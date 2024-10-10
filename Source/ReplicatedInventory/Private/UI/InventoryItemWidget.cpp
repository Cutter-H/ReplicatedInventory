// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/InventoryItemWidget.h"
#include "Item/ItemDataComponent.h"
#include "UI/ReplicatedDragHolder.h"

#include "Components/SizeBox.h"
#include "Components/SizeBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"

#include "Blueprint/WidgetTree.h"

void UInventoryItemWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SizeBox_Root)
	{
		SizeBox_Root->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (IsDesignTime())
		{
			if (!GetClass()->IsChildOf(UInventoryItemWidget::StaticClass()))
			{
				SizeBox_Root->SetWidthOverride(BoxSize.X);
				SizeBox_Root->SetHeightOverride(BoxSize.Y);
			}
			else
			{
				BoxSize.X = SizeBox_Root->GetWidthOverride();
				BoxSize.Y = SizeBox_Root->GetHeightOverride();
			}
		}
		if (Overlay_Item)
		{
			if (USizeBoxSlot* overlayAsSlot = Cast<USizeBoxSlot>(SizeBox_Root->AddChild(Overlay_Item)))
			{
				overlayAsSlot->SetPadding(0.f);
				overlayAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
				overlayAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			}
			if (ItemImage)
			{
				if (UOverlaySlot* imageAsSlot = Overlay_Item->AddChildToOverlay(ItemImage))
				{
					imageAsSlot->SetPadding(5.f);
					imageAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
					imageAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
				}
			}
			if (UIText)
			{
				if (UOverlaySlot* textAsSlot = Overlay_Item->AddChildToOverlay(UIText))
				{
					textAsSlot->SetPadding(5.f);
					textAsSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
					textAsSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
				}
				if (!GetClass()->IsChildOf(UInventoryItemWidget::StaticClass()))
				{
					FSlateFontInfo font = UIText->GetFont();
					font.Size = FontSize;
					font.OutlineSettings.OutlineSize = 1;
					UIText->SetFont(font);
					UIText->SetJustification(ETextJustify::Right);
				}
			}
		}
	}
	/**/
}

void UInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	OnItemQuantityUpdate(0,0);
	SetIsFocusable(true);
}

void UInventoryItemWidget::OnSlotButtonPressed_Implementation()
{
	OnSlotButtonPressed_Internal();
}
void UInventoryItemWidget::OnSlotButtonPressed_Internal()
{

}

void UInventoryItemWidget::OnSlotButtonHovered_Implementation()
{
	OnSlotButtonHovered_Internal();
}
void UInventoryItemWidget::OnSlotButtonHovered_Internal()
{

}

void UInventoryItemWidget::OnSlotButtonUnhovered_Implementation()
{
	OnSlotButtonUnhovered_Internal();
}
void UInventoryItemWidget::OnSlotButtonUnhovered_Internal()
{

}

void UInventoryItemWidget::OnItemQuantityUpdate_Implementation(int oldQuantity, int newQuantity) {
	OnItemQuantityUpdate_Internal(oldQuantity, newQuantity);
}
void UInventoryItemWidget::OnItemQuantityUpdate_Internal(int oldQuantity, int newQuantity) {
	if (!ItemDataAsset)return;

	if (ItemImage)
	{
		ItemImage->SetBrushFromMaterial(ItemDataAsset->GetImage());
	}
	if (UIText)
	{
		UIText->SetText(ItemDataAsset->GetGridDisplayText());
	}
}

void UInventoryItemWidget::SetItemInfo(UItemDataComponent* newItemData) {	
	if (ItemDataAsset)
	{
		ItemDataAsset->OnQuantityChange.RemoveDynamic(this, &UInventoryItemWidget::OnItemQuantityUpdate);
	}
	if (IsValid(newItemData))
	{
		ItemDataAsset->OnQuantityChange.AddDynamic(this, &UInventoryItemWidget::OnItemQuantityUpdate);
		SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
	ItemDataAsset = newItemData;
	OnItemQuantityUpdate(0, ItemDataAsset->GetQuantity());
}

void UInventoryItemWidget::SetSize(FVector2D size) {
	if(!IsDesignTime())
		BoxSize = size;
	if (!SizeBox_Root) return;
	UpdateSize();
	/*
	if (Item)
	{
		SizeBox_Root->SetWidthOverride(size.X * Item->Size.Width);
		SizeBox_Root->SetHeightOverride(size.Y * Item->Size.Height);
	}
	else
	{
		if (IsDesignTime())
		{
			SizeBox_Root->SetWidthOverride(BoxSize.X);
			SizeBox_Root->SetHeightOverride(BoxSize.Y);
		}
		else
		{
			SizeBox_Root->SetWidthOverride(size.X);
			SizeBox_Root->SetHeightOverride(size.Y);

		}
	}*/
}

void UInventoryItemWidget::SetTint(FLinearColor tint) {
	if (ItemImage) {
		ItemImage->SetBrushTintColor(tint);

	}
	if (UIText) {
		UIText->SetColorAndOpacity(FSlateColor(tint));

	}
}

void UInventoryItemWidget::SetFontSize(int newFontSize) {
	FontSize = newFontSize;
	if (IsValid(UIText)) {
		FSlateFontInfo font = UIText->GetFont();
		font.Size = FontSize;
		UIText->SetFont(font);
	}
}

void UInventoryItemWidget::UpdateWidget() {
	UpdateSize();
	UpdateVisibility();

}

void UInventoryItemWidget::UpdateSize() {
	if (IsValid(ItemDataAsset)) {
		FItemGridSize size = ItemDataAsset->GetSize();
		SizeBox_Root->SetWidthOverride(BoxSize.X * size.Width);
		SizeBox_Root->SetHeightOverride(BoxSize.Y * size.Height);
	}
	else {
		SizeBox_Root->SetWidthOverride(BoxSize.X);
		SizeBox_Root->SetHeightOverride(BoxSize.Y);
	}
}

void UInventoryItemWidget::UpdateVisibility() {
	if (IsValid(ItemDataAsset)) {
		SetVisibility(ESlateVisibility::HitTestInvisible);

	}
	else {
		SetVisibility(ESlateVisibility::Hidden);
	}
}
