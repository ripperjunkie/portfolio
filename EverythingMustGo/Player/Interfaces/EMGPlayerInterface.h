// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "EverythingMustGo/Entities/ShoppingItem.h"
#include "UObject/Interface.h"
#include "EverythingMustGo/Public/EMGUtils.h"


#include "EMGPlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UEMGPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class EVERYTHINGMUSTGO_API IEMGPlayerInterface
{
	GENERATED_BODY()

	
public:
	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void AddShoppingItemToInventory(FItemsInInventory ItemToAdd);

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void StartedCheckingOut();
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void CompletedCheckingOut();
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void InterruptedCheckingOut();

	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	bool HasItems();	

	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void OnGettingRammed();
};
