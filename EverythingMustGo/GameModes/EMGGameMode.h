// Copyright (c) 2023 @ Runaway Concepts - Rafael Zagolin

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EMGUtils.h"

#include "EMGGameMode.generated.h"


class AEMGPlayerController;
class UEMGGameInstance;
class AEMGBasePlayer;
class APlayerStart;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartedStartup);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMatchEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEveryoneReady);

UCLASS()
class EVERYTHINGMUSTGO_API AEMGGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEMGGameMode();

	UPROPERTY(BlueprintReadOnly)
	AEMGPlayerController* ServerPlayerController;

	UFUNCTION(BlueprintCallable)
	void SetupMatch();
	
	UFUNCTION(BlueprintCallable)
	void OnLoadMapStartStartup();

	UFUNCTION(BlueprintCallable)
	void StartCountdown();

	UFUNCTION(BlueprintCallable)
	void CustomEndMatch();

	/* Config match time in minutes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MatchTimeMinutes = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = MatchConfig)
	FTimerHandle MatchTimer;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle Match_TimerHandle; // timer to setup the match

	UPROPERTY(BlueprintReadWrite, Category = MatchConfig)
	FTimerHandle StartupTimer_TimerHandle;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle SetupMatch_TimerHandle; // timer to setup the match
	FTimerHandle SmallDelay_TimerHandle;

	UPROPERTY(BlueprintAssignable)
	FOnEveryoneReady OnEveryoneReadyDelegate;

	UFUNCTION(BlueprintCallable)
	void TravelToGameMap(FString MapToTravel);

	UFUNCTION(BlueprintCallable)
	void LoadPlayerScores();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/* Config startup time in seconds */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 StartupTimeSeconds = 3.f;

	/* Delegate for when match starts */
	UPROPERTY(BlueprintAssignable, Category = Delegates)
	FMatchStarted MatchStartedDelegate;

	/* Delegate for when match ends */
	UPROPERTY(BlueprintAssignable, Category = Delegates)
	FMatchEnded MatchEndedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Delegates)
	FStartedStartup StartStartupDelegate;


	void ImGuiWindow();

	/************* MATCH TIMER LOGIC *************/
	UPROPERTY(EditAnywhere)
	float InitialDelayToStartMatch = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MatchMinutesLength = 3;
	UPROPERTY(BlueprintReadOnly)
	int32 MatchMinutesLength_Conversion;

	UFUNCTION(BlueprintCallable)
	void UpdateMatch();

	// DEPRECATED
	UFUNCTION()
	FString GetWinner();

	std::tuple<FString, int32, APlayerState*> GetWinnerInfo();

	UPROPERTY(BlueprintReadWrite)
	uint32 bMatchRunning : 1;

private:
	UPROPERTY()
	TMap<APlayerStart*, bool> PlayerStartTable; // check if player start has been assigned to a player controller
};
