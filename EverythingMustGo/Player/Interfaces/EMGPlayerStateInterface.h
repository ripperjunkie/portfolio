// Copyright (c) 2023 @ Runaway Concepts
#pragma once

#include "CoreMinimal.h"
#include "EMGUtils.h"


#include "EMGPlayerStateInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UEMGPlayerStateInterface : public UInterface
{
public:
	GENERATED_BODY()
};


class EVERYTHINGMUSTGO_API IEMGPlayerStateInterface
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	FVector GetControlledPawnLocation();
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	FVector GetControlledPawnForwardDirection();
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	FPlayerInfo GetPlayerInfo();
	
};