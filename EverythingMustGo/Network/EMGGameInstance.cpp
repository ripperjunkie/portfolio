// Copyright (c) 2023 @ Runaway Concepts - Rafael Zagolin, Pedro Perez


#include "EMGGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "EverythingMustGo/Player/PlayerControllers/EMGPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetStringLibrary.h"
#include "EMGUtils.h"
#include "GameFramework/GameMode.h"

IOnlineSessionPtr UEMGGameInstance::SessionInterface = nullptr;


UEMGGameInstance::UEMGGameInstance()
{
}
void UEMGGameInstance::Init()
{
	Super::Init();

	if (const IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
		
		SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UEMGGameInstance::OnCreateSessionComplete);
		SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UEMGGameInstance::OnJoinSessionComplete);
		SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UEMGGameInstance::OnDestroySessionComplete);
	}
}

void UEMGGameInstance::CreateSession(FName _SessionName, int32 MaxPlayers, bool bUseLAN)
{
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bIsLANMatch = bUseLAN;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.NumPublicConnections = MaxPlayers;

	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController)
	{
		return;
	}
	CustomSessionName = _SessionName;
	SessionInterface->CreateSession(0, FName(_SessionName), SessionSettings);
}


void UEMGGameInstance::DisconnectFromSession(APlayerController* PC)
{
	const UWorld* world = GetWorld();

	if(!world)
	{
		UE_LOG(LogTemp,Warning,TEXT("Invalid World!"));
		return;
	}

	if (!SessionInterface)
	{
		UE_LOG(LogTemp,Warning,TEXT("Invalid SessionInterface!"));
		return;
	}

	// Client request to exit session
	if (!GetWorld()->IsServer())
	{
		// Called on the client if he wants to disconnect from session
		if(PC)
		{
			PC->ClientReturnToMainMenuWithTextReason(NSLOCTEXT("GameMessages", "UserQuits","User Quit the Game"));			
		}
		return;
	}
	
	// All things below will be called on the Server
	// Disconnect session and all players
	const FNamedOnlineSession* ExistingSessionName = SessionInterface->GetNamedSession(CustomSessionName);

	TArray<AActor*> PlayerControllers;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerController::StaticClass(), PlayerControllers);
	for (auto NewPlayer : PlayerControllers)
	{
		if(APlayerController* clientPC = Cast<APlayerController>(NewPlayer))
		{			
			if(clientPC->GetNetMode() == NM_Client)
			{
				clientPC->ClientReturnToMainMenuWithTextReason(NSLOCTEXT("GameMessages", "Server shuts down","server shuts down match"));			
			}
			UE_LOG(LogTemp,Warning,TEXT("%hs"), __FUNCTION__);
		}
		else
		{
			UE_LOG(LogTemp,Warning,TEXT("%s"), *FString("Failed casting"));
		}
	}

	
	if(ExistingSessionName)
	{
		SessionInterface->DestroySession(CustomSessionName);
	}
}

#pragma region Session Callbacks
void UEMGGameInstance::OnCreateSessionComplete(FName SessionName, bool bSucceeded)
{
	UE_LOG(LogTemp, Warning, TEXT("CustomSessionName: %s"), *SessionName.ToString());
	if (bSucceeded)
	{
		GetWorld()->ServerTravel(MapToOpen + "?listen");
	}
}

void UEMGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete, CustomSessionName: %s"), *SessionName.ToString());

	if (APlayerController* pc = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FString joinAddress = "";
		SessionInterface->GetResolvedConnectString(SessionName, joinAddress);
		if (joinAddress != "")
		{
			UE_LOG(LogTemp, Warning, TEXT("JoiningServer"));
			pc->ClientTravel(joinAddress, ETravelType::TRAVEL_Absolute);
		}
	}
}

void UEMGGameInstance::OnDestroySessionComplete(FName SessionName, bool Successful)
{
	if(AGameModeBase* gm = UGameplayStatics::GetGameMode(GetWorld()))
	{
		gm->ReturnToMainMenuHost();
		UE_LOG(LogTemp,Warning,TEXT("%s"), *FString("gm->ReturnToMainMenuHost()"));
	}
}
#pragma endregion