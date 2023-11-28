// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "EMGUtils.h" 

#include "EMGLobbyGameMode.generated.h"

class UEMGGameInstance;

UCLASS()
class EVERYTHINGMUSTGO_API AEMGLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEMGLobbyGameMode();
	
	UFUNCTION(BlueprintCallable)
	void TravelToGameMap();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPlayerInfo> PlayerInfos;
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(BlueprintReadOnly)
	UEMGGameInstance* GameInstanceRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapToTravel;
	
	
};
