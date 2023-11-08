// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "EverythingMustGo/Player/Interfaces/EMGPlayerStateInterface.h"
#include "EMGUtils.h"


#include "EMGPlayerState.generated.h"


UCLASS()
class EVERYTHINGMUSTGO_API AEMGPlayerState : public APlayerState, public IEMGPlayerStateInterface
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnScoreUpdate_BPIE(float newScore);
	

	virtual FVector GetControlledPawnLocation_Implementation() override;
	virtual FVector GetControlledPawnForwardDirection_Implementation() override;
	virtual FPlayerInfo GetPlayerInfo_Implementation() override;

protected:
	/* Player Information assigning by the server */
	UPROPERTY(BlueprintReadWrite)
	FPlayerInfo PlayerInfo;
	
};
