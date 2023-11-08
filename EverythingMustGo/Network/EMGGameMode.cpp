// Copyright (c) 2023 @ Runaway Concepts - Rafael Zagolin, Pedro Perez


#include "EverythingMustGo/Public/EMGGameMode.h"

#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Engine.h"

#include <imgui.h>

#include "EverythingMustGo/EMGConsoleManager.h"
#include "EverythingMustGo/GameInstances/EMGGameInstance.h"
#include "EverythingMustGo/GameStates/EMGGameState.h"
#include "EverythingMustGo/Player/PlayerControllers/EMGPlayerController.h"
#include "Runtime/Engine/Classes/Engine/World.h"

#define LOG(time, text) if(GEngine) \
GEngine->AddOnScreenDebugMessage(-1, time, FColor::Green, FString(text));


// Sets default values
AEMGGameMode::AEMGGameMode()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bStartPlayersAsSpectators = false;
	bPauseable = false;
	
	// PlayerInfos.Add({false, FColor::Cyan, ""});
	// PlayerInfos.Add({false, FColor::Red, ""});
	// PlayerInfos.Add({false, FColor::Yellow, ""});
	// PlayerInfos.Add({false, FColor::Green, ""});
}

// Called when the game starts or when spawned
void AEMGGameMode::BeginPlay()
{
	Super::BeginPlay();

	
	/* Update custom match state */
	bMatchRunning = 0;

	// set match state to not running
	if(AEMGGameState* _gameState  = GetGameState<AEMGGameState>())
	{
		_gameState->bMatchRunning = bMatchRunning;
	}


	// Here we get the information from the game instance because we set it in another level (lobby level)
	if (UEMGGameInstance* GameInstance = Cast<UEMGGameInstance>(UGameplayStatics::GetGameInstance(GetWorld())))
	{
		if (GameInstance->bShouldRetrieveGameModeInfo)
		{
			MatchTimeMinutes = GameInstance->MatchMinutesLength;
		}
	}

	// TODO: Check if all player controllers are possessing their pawns
	// Compare if all player controllers are equal to possessing their pawns
	// while(NotEveryoneIsReady)
	// {
	// 	// run
	// 	if(true)
	// 	{
	// 		OnEveryoneReadyDelegate.Broadcast();
	// 	}
	// }
	
	

	// Start countdown timer
	GetWorld()->GetTimerManager().SetTimer(
		SmallDelay_TimerHandle, [&]()	
	    {
		    // Do time conversions
		    MatchMinutesLength_Conversion = MatchTimeMinutes * 60;
			
			// call to start match
			OnLoadMapStartStartup();   
	    }
	    , InitialDelayToStartMatch, false, InitialDelayToStartMatch);

}

void AEMGGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Invalidate all timers
	if(Match_TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(Match_TimerHandle);
	}
	if(MatchTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(MatchTimer);
	}

	if(StartupTimer_TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(StartupTimer_TimerHandle);
	}

	if(SetupMatch_TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SetupMatch_TimerHandle);
	}

	if(SmallDelay_TimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SmallDelay_TimerHandle);
	}
	
}

// Called every frame
void AEMGGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ImGuiWindow();
}

void AEMGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
#if UE_BUILD_DEVELOPMENT
	// for now this will allow cheats for every new player controller that logs in the match, but it's
	// for debugging purposes only!
	AllowCheats(NewPlayer);
	NewPlayer->AddCheats(true);
#endif

	// Notify game state a new player controller has logged in
	if(AEMGGameState* _gameState  = GetGameState<AEMGGameState>())
	{
		_gameState->OnPostLogin.Broadcast(NewPlayer);
	}
}

void AEMGGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (GameInstanceRef)
	{
		// Reduce players from connected session
		GameInstanceRef->PlayerAmountInMatch--;
	}

	// Notify game state a player has logged out
	if(AEMGGameState* _gameState  = GetGameState<AEMGGameState>())
	{
		_gameState->OnLogout.Broadcast(Exiting);
	}
}

// Window for debugging stuff
void AEMGGameMode::ImGuiWindow()
{
	if (!UEMGConsoleManager::bToggleWindowGUIDebug)
	{
		return;
	}

	// Get all players controllers
	TArray<AActor*> PlayerControllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), PlayerControllers);

	if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		{
			ImGui::Begin("GameMode Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::Button("Start Startup Match"))
			{
				UE_LOG(LogTemp, Display, TEXT("Clicked 'Start Startup Match'."));
				StartCountdown();
			}
			if (ImGui::Button("End Match"))
			{
				UE_LOG(LogTemp, Display, TEXT("Clicked 'End Match'."));

				CustomEndMatch();
			}

			{
				if (GameInstanceRef)
				{
					// Convert the int to FString
					FString IntAsString = FString::Printf(TEXT("%d"), GameInstanceRef->PlayerAmountInMatch);


					// Convert FString to const char*
					const char* CharPtr = TCHAR_TO_UTF8(*IntAsString);

					ImGui::LabelText(CharPtr, "Players On Match: ");
				}
			}


			{
				// Convert the int to FString
				FString IntAsString = FString::Printf(TEXT("%d"), PlayerControllers.Num());

				// Convert FString to const char*
				const char* CharPtr = TCHAR_TO_UTF8(*IntAsString);

				ImGui::LabelText(CharPtr, "Players Connected: ");
				// players that currently connected to match (entered the level)
			}

			{
				// Convert the int to FString
				FString IntAsString = FString::Printf(TEXT("%d"), PlayerStartTable.Num());

				// Convert FString to const char*
				const char* CharPtr = TCHAR_TO_UTF8(*IntAsString);

				ImGui::LabelText(CharPtr, "Players Starts Found: ");
				// players that currently connected to match (entered the level)				
			}
			{
				// Convert the int to FString
				FString IntAsString = FString::Printf(TEXT("%d"), NumTravellingPlayers);

				// Convert FString to const char*
				const char* CharPtr = TCHAR_TO_UTF8(*IntAsString);

				ImGui::LabelText(CharPtr, "NumTravellingPlayers: ");
				// players that currently connected to match (entered the level)				
			}
			//NumTravellingPlayers

			ImGui::End();
		}
	}
}

FString AEMGGameMode::GetWinner()
{
	// Get all players controllers
	TArray<AActor*> PlayerControllers;
	if (!GetWorld())
	{
		return ("Invalid World");
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), PlayerControllers);

	if (PlayerControllers.Num() != 0)
	{
		APlayerState* WinnerPC = Cast<APlayerController>(PlayerControllers[0])->GetPlayerState<APlayerState>();
		for (auto& Item : PlayerControllers)
		{
			if (Item)
			{
				if (APlayerState* PlayerStateLocalRef = Cast<APlayerController>(Item)->GetPlayerState<APlayerState>())
				{
					if (PlayerStateLocalRef)
					{
						if (PlayerStateLocalRef->GetScore() > WinnerPC->GetScore())
						{
							WinnerPC = PlayerStateLocalRef;
						}
					}
				}
			}
		}

		return WinnerPC->GetPlayerName();
	}

	return ("No players found");
}

// Returns the winner name, winner score. Theoretically now we could remove both the string and int32 and just return
// the player state, but let's keep it like this as an example of tuple use case.
std::tuple<FString, int32, APlayerState*> AEMGGameMode::GetWinnerInfo()
{
	// attribute default value
	std::tuple<FString, int32, APlayerState*> value = std::make_tuple<FString, int32>(FString(""), 0, nullptr);
	
	// Get all players controllers
	TArray<AActor*> PlayerControllers;
	if (!GetWorld())
	{
		return value;
	}

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), PlayerControllers);
	APlayerState* WinnerPC = nullptr;
	if (PlayerControllers.Num() != 0)
	{
		WinnerPC = Cast<APlayerController>(PlayerControllers[0])->GetPlayerState<APlayerState>();
		for (auto& Item : PlayerControllers)
		{
			if (Item)
			{
				if (APlayerState* PlayerStateLocalRef = Cast<APlayerController>(Item)->GetPlayerState<APlayerState>())
				{
					if (PlayerStateLocalRef)
					{
						if (PlayerStateLocalRef->GetScore() > WinnerPC->GetScore())
						{
							WinnerPC = PlayerStateLocalRef;
						}
					}
				}
			}
		}
	}

	
	std::get<0>(value) = WinnerPC->GetPlayerName(); // assign winner player nickname
	std::get<1>(value) = WinnerPC->GetScore(); // assign winner score
	std::get<2>(value) = WinnerPC; // assign winner player state
	return value;
}

void AEMGGameMode::OnLoadMapStartStartup()
{
	// Start timer
	StartStartupDelegate.Broadcast();
	GetWorld()->GetTimerManager().SetTimer(
		StartupTimer_TimerHandle, [&]()	
		{	
			// call to start match
			StartCountdown();					   
		}
		, 1.f, true, 0.f);
}


void AEMGGameMode::StartCountdown()
{
	const UWorld* CurrentWorld = GetWorld();
	UE_LOG(LogTemp, Display, TEXT(__FUNCTION__));
	
	if(	StartupTimeSeconds - 1 >= -1)
	{
		// update startup timer on game state
		if (AEMGGameState* _GameState = Cast<AEMGGameState>( UGameplayStatics::GetGameState( CurrentWorld ) ) )
		{		
			_GameState->StartupTimer = StartupTimeSeconds;
		}
		StartupTimeSeconds--;
		
	}
	else
	{		
		CurrentWorld->GetTimerManager().ClearTimer(StartupTimer_TimerHandle);
		UE_LOG(LogTemp, Display, TEXT("Request to EMG_StartMatch"));		
		UpdateMatch();
	}

	if(StartupTimeSeconds <= 1)
	{
		/* Update custom match state */
		bMatchRunning = 1;
		
		if(AEMGGameState* _gameState  = GetGameState<AEMGGameState>())
		{
			_gameState->bMatchRunning = bMatchRunning;
		}
	}
}

void AEMGGameMode::UpdateMatch()
{
	// broadcast to everyone subscribed to this delegate that match has started
	MatchStartedDelegate.Broadcast();
	
	// Notify game state about match start
	if(AEMGGameState* _gameState  = GetGameState<AEMGGameState>())
	{
		_gameState->MatchStartedDelegate.Broadcast();
	}
	
	GetWorld()->GetTimerManager().SetTimer(Match_TimerHandle, [&]()
	{
		// Notifying all clients about the match timer
		if(IsValid(GetWorld()))
		{
			const UWorld* CurrentWorld = GetWorld();
			if (AEMGGameState* GameState = Cast<AEMGGameState>( UGameplayStatics::GetGameState( CurrentWorld ) ) )
			{
				auto t = GetWinnerInfo();	
				GameState->MatchTimer = MatchMinutesLength_Conversion;

				// use of C++ tuples, pretty cool :D. Here we let the game state know about the winner info
				GameState->Winner = std::get<0>( t);			
				GameState->WinnerScore = std::get<1>( t);
				GameState->WinnerPlayerState = std::get<2>( t);
			}

			if (MatchMinutesLength_Conversion - 1 > 0)
			{
				MatchMinutesLength_Conversion--;
			}
			else // END OF MATCH
			{
				MatchMinutesLength_Conversion = 0;
				GetWorld()->GetTimerManager().ClearTimer(Match_TimerHandle);

				if (UEMGGameInstance* GameInstance = Cast<UEMGGameInstance>(
					UGameplayStatics::GetGameInstance(CurrentWorld)))
				{
					// DEPRECATED, we are not really using it
					/* We are letting the game instance know about the winner in case we travel to other maps and we don't want to
					lose information on the transition. It'd be better to have a structure that contains all the data we want to save,
					but it should be fine for now.*/
					GameInstance->Winner = GetWinner();

					// Update MatchTimeLeft in server player controller only
					if (ServerPlayerController)
					{
						ServerPlayerController->MatchTimeLeft = MatchMinutesLength_Conversion;
						ServerPlayerController->WinnerName = GameInstance->Winner;
					}
				}

				// broadcast to everyone subscribed to this delegate that match has ended
				MatchEndedDelegate.Broadcast();
			}
		}
	}
	, 1.f, true, 0.f);
}

/* CURRENTLY NOT BEING USED, MIGHT GET DEPRECATED! */
void AEMGGameMode::SetupMatch()
{
	// Get all players controllers
	TArray<AActor*> PlayerControllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), PlayerControllers);

	// Check if we have the same amount of pc as we had in the lobby
	if (PlayerControllers.Num() < GameInstanceRef->PlayerAmountInMatch)
	{
		UE_LOG(LogTemp, Warning, TEXT("Some player(s) is still travelling on the map..."));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("All players arrived, let's check if all of them are possessing a pawn..."));


	for (auto NewPlayer : PlayerControllers)
	{
		// Get all player starts within level
		UE_LOG(LogTemp, Warning, TEXT("Get all player starts within level"));

		for (TTuple<APlayerStart*, bool>& item : PlayerStartTable)
		{
			// check if it's not occupied
			bool bIsOccupied = item.Value;
			UE_LOG(LogTemp, Warning, TEXT("bIsOccupied: %s"), bIsOccupied ? TEXT("true") : TEXT("false"));

			if (!bIsOccupied)
			{
				// spawn default pawn
				const FVector SpawningLocation = item.Key->GetActorLocation();
				const FRotator SpawningRotation = item.Key->GetActorRotation();

				AActor* SpawnedPawn = GetWorld()->SpawnActor<AActor>(DefaultPawnClass, SpawningLocation,
				                                                     SpawningRotation);
				UE_LOG(LogTemp, Display, TEXT("Spawning player pawn at its proper player start (arbitrarily)"));

				// Tell new player controller to posses spawned pawn...
				APlayerController* PC = Cast<APlayerController>(NewPlayer);
				PC->Possess(Cast<APawn>(SpawnedPawn));
				//PC->SetIgnoreMoveInput(true);
				item.Value = true; // set the player start as occupied
				break;
			}
		}
	}


	for (AActor* item : PlayerControllers)
	{
		if (const APlayerController* PC = Cast<APlayerController>(item))
		{
			// check if it's not possessing a pawn
			if (!PC->GetPawn())
				return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("All pcs are possessing a pawn..."));

	// Pausing timer so we don't call this function again after all pawns have spawned
	GetWorld()->GetTimerManager().PauseTimer(SetupMatch_TimerHandle);


	GetWorld()->GetTimerManager().SetTimer(SmallDelay_TimerHandle, [&]()
	                                       {
		                                       StartCountdown();
	                                       }
	                                       , 1.f, false, 1.f);
}

void AEMGGameMode::CustomEndMatch()
{
	// For now, let's remove input from all players when match ends
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		//It->SetIgnoreMoveInput(true);
	}

	UE_LOG(LogTemp, Display, TEXT("Request to EndMatch."));


	// Clear timer
	GetWorld()->GetTimerManager().ClearTimer(MatchTimer);

	// broadcast to everyone subscribed to this delegate that match has ended
	MatchEndedDelegate.Broadcast();
}

