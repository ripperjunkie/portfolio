// Copyright (c) 2023 @ Runaway Concepts - Rafael Zagolin, Pedro Perez


#include "EMGGameState.h"

#include "EverythingMustGo/Player/PlayerControllers/EMGPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"


// Sets default values
AEMGGameState::AEMGGameState()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PlayerInfos.Add({false, FColor::Cyan, ""});
	PlayerInfos.Add({false, FColor::Red, ""});
	PlayerInfos.Add({false, FColor::Yellow, ""});
	PlayerInfos.Add({false, FColor::Green, ""});
}

// Called when the game starts or when spawned
void AEMGGameState::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEMGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME( AEMGGameState, MatchTimer );
	DOREPLIFETIME( AEMGGameState, Winner );
	DOREPLIFETIME( AEMGGameState, WinnerScore );
	DOREPLIFETIME(AEMGGameState, PlayerInfos);
	DOREPLIFETIME( AEMGGameState, StartupTimer );
	DOREPLIFETIME( AEMGGameState, bMatchRunning );
	DOREPLIFETIME( AEMGGameState, WinnerPlayerState );	
}

void AEMGGameState::Server_FindAvailableColor_Implementation(APlayerController* _PlayerController, FPlayerInfo PlayerInfo)
{
	Multicast_FindAvailableColor(_PlayerController, PlayerInfo);
}

void AEMGGameState::Multicast_FindAvailableColor_Implementation(APlayerController* _PlayerController, FPlayerInfo PlayerInfo)
{
	/* Assign player info on player controller. Basically finds a color that hasn't been occupied yet and assign it to
	the player*/
	if(AEMGPlayerController* playerController = Cast<AEMGPlayerController>(_PlayerController))
	{
		playerController->PlayerInfo = PlayerInfo;
	}
	
}


