// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin


#include "EMGPlayerState.h"
#include "EverythingMustGo/Player/PlayerControllers/EMGPlayerController.h"

void AEMGPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	OnScoreUpdate_BPIE(GetScore());
}

FVector AEMGPlayerState::GetControlledPawnLocation_Implementation()
{
	FVector pawnLocation;
	if(GetPawn())
	{
		pawnLocation = GetPawn()->GetActorLocation();
	}
	return pawnLocation;
}

FVector AEMGPlayerState::GetControlledPawnForwardDirection_Implementation()
{
	FVector pawnForwardDir;
	if(GetPawn())
	{
		pawnForwardDir = GetPawn()->GetActorForwardVector();
	}
	return pawnForwardDir;
}

FPlayerInfo AEMGPlayerState::GetPlayerInfo_Implementation()
{
	if(AEMGPlayerController* pc =Cast<AEMGPlayerController>(GetOwner()) )
	{
		return pc->PlayerInfo;
	}	
	return FPlayerInfo();
}
