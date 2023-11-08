// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "EMGUtils.h"

#include "EMGGameState.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostLogin, APlayerController* , PlayerController);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLogout, AController*, Exiting);

class AEMGBasePlayer;

UCLASS()
class EVERYTHINGMUSTGO_API AEMGGameState : public AGameState
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEMGGameState();
	
	UPROPERTY(BlueprintReadWrite, Replicated)
	int32 MatchTimer;	
	UPROPERTY(BlueprintReadWrite, Replicated)
	int32 StartupTimer;


	// It'd be better to have a struct that holds info about the winner but for now we'll do it on this way
	/* Store winner nickname */
	UPROPERTY(BlueprintReadWrite, Replicated)
	FString Winner;
	UPROPERTY(BlueprintReadWrite, Replicated)
	int32 WinnerScore;
	UPROPERTY(BlueprintReadOnly, Replicated)
	APlayerState* WinnerPlayerState;
	
	
	/* Store winner nickname */
	UPROPERTY(BlueprintReadOnly, Replicated)
	uint32 bMatchRunning : 1;

	 UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	 TArray<FPlayerInfo> PlayerInfos;

	UFUNCTION(Server,Reliable, BlueprintCallable)
	void Server_FindAvailableColor(APlayerController* _PlayerController, FPlayerInfo PlayerInfo);	
	UFUNCTION(NetMulticast,Reliable, BlueprintCallable)
	void Multicast_FindAvailableColor(APlayerController* _PlayerController, FPlayerInfo PlayerInfo);


	virtual void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const;

	UPROPERTY(BlueprintAssignable)
	FOnPostLogin OnPostLogin;
	UPROPERTY(BlueprintAssignable)
	FOnLogout OnLogout;
	/* Delegate for when match starts */
	UPROPERTY(BlueprintAssignable, Category = Delegates)
	FMatchStarted MatchStartedDelegate;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
