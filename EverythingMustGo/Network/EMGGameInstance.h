// Copyright (c) 2023 @ Runaway Concepts - Rafael Zagolin, Pedro Perez


#pragma once

// Import necessary headers and libraries for the code.
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "EverythingMustGo/Player/PlayerStates/EMGPlayerState.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "EMGGameInstance.generated.h"

// Define a shared pointer type for online sessions.
typedef TSharedPtr<IOnlineSession, ESPMode::ThreadSafe> IOnlineSessionPtr;

UCLASS()
class EVERYTHINGMUSTGO_API UEMGGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UEMGGameInstance();

	UFUNCTION(BlueprintCallable)
	void DisconnectFromSession(APlayerController* PC);

	UFUNCTION(BlueprintPure)
	class UOnlineSession* ConnectedOnlineSession() const
	{
		return GetOnlineSession();
	}
	
	static IOnlineSessionPtr SessionInterface;
	
	// Name of the custom session.
	UPROPERTY(BlueprintReadWrite)
	FName CustomSessionName = FName("DummySession");

	/* We'll use this information for when transitioning between levels */
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerAmountInMatch = 0;

	// Store Name of the winner if we want to transition between levels without losing information.
	UPROPERTY(BlueprintReadWrite)
	FString Winner;

	// Flag to retrieve game mode information.
	UPROPERTY(BlueprintReadWrite)
	bool bShouldRetrieveGameModeInfo;

	/* Saving on game instance so it's transferable between levels*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MatchMinutesLength;

	// This function to check if we are still in session
	UFUNCTION(BlueprintPure, Category="Network helper functions")
	FORCEINLINE bool IsStillInSession()
	{
		APlayerController* PC = GetFirstLocalPlayerController();
		if(!PC)
		{
			return false;
		}
		
		APlayerState* playerState = PC->GetPlayerState<APlayerState>();
		if(!playerState)
		{
			return false;
		}
		
		FUniqueNetIdRepl PlayerID = playerState->GetUniqueId();

		if(SessionInterface)
		{
			return SessionInterface->IsPlayerInSession(CustomSessionName, *PlayerID.GetUniqueNetId());
		}
		
		return false;
	}

	
protected:

	/* Stored found sessions */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	virtual void Init() override;

	// Callbacks for session creation 
	virtual void OnCreateSessionComplete(FName SessionName, bool bSucceeded);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnDestroySessionComplete(FName SessionName, bool Successful);

	UFUNCTION(BlueprintCallable)
	void CreateSession(FName _SessionName, int32 MaxPlayers, bool bUseLAN = true);
	
	UPROPERTY(EditDefaultsOnly)
	FString MapToOpen;

};
