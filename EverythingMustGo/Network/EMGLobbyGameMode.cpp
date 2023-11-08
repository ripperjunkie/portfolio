// Copyright (c) 2023 @ Runaway Concepts - Rafael Zagolin, Pedro Perez


#include "EMGLobbyGameMode.h"

#include "EverythingMustGo/GameInstances/EMGGameInstance.h"
#include "EverythingMustGo/GameStates/EMGGameState.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AEMGLobbyGameMode::AEMGLobbyGameMode()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseSeamlessTravel = true;
}

// Called when the game starts or when spawned
void AEMGLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	GameInstanceRef = Cast<UEMGGameInstance>(GetWorld()->GetGameInstance());
}

void AEMGLobbyGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Set the players amount in current lobby for game instance
	if (GameInstanceRef)
	{
		GameInstanceRef->PlayerAmountInMatch = GetNumPlayers();
	}
}

// Called every frame
void AEMGLobbyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// if(GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green,  *GetMatchState().ToString());
	// }

}

void AEMGLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Notify the game state 
	if (AEMGGameState* gameState = Cast<AEMGGameState>(UGameplayStatics::GetGameState(GetWorld())))
	{
		gameState->OnPostLogin.Broadcast(NewPlayer);
		for (auto& PlayerInfo : PlayerInfos)
		{
			if (!PlayerInfo.bOccupied)
			{
				PlayerInfo.bOccupied = true;
				PlayerInfo.PlayerID = NewPlayer->PlayerState->GetPlayerName();
				gameState->Server_FindAvailableColor(NewPlayer, PlayerInfo);
				gameState->PlayerInfos = PlayerInfos;
				break;
			}
		}
	}
}

void AEMGLobbyGameMode::TravelToGameMap()
{
	// cache world
	const UWorld* world = GetWorld();
	if (!world)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid world."));
		return;
	}
	
	// cache player controller	
	APlayerController* pc = world->GetFirstPlayerController();
	if (!pc)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid player controller."));
	}

	// Let the game instance know how many players we currently have
	checkf(GameInstanceRef, TEXT("Invalid game instance for setting amount of players"))
	{
		GameInstanceRef->PlayerAmountInMatch = GetNumPlayers();	
	}
	
	// command to travel to new map
	pc->ConsoleCommand("servertravel " + MapToTravel);
	UE_LOG(LogTemp, Warning, TEXT("Requested to travel map with %d amount of players"), GetNumPlayers());
}
