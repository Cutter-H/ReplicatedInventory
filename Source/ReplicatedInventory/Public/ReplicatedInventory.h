// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

REPLICATEDINVENTORY_API DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

class FReplicatedInventoryModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
