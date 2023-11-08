// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EverythingMustGo/Entities/EMG_ObstacleHazard.h"
#include "EverythingMustGo/Entities/EMG_SlipperyHazzard.h"
#include "Math/UnrealMathUtility.h"
#include "EverythingMustGo/Player/EMGBasePlayer.h"
#include "EMGPowerup.generated.h"

UENUM(BlueprintType)
enum class EPowerUpType : uint8
{
	None = 0,
	Slippery = 1,
	Gooey = 2,
	SpeedBoost = 3,
	InfiniteInventory = 4,
	Shield = 5,
	Count UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EPowerUpType,EPowerUpType::Count);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EVERYTHINGMUSTGO_API UEMGPowerup : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEMGPowerup();

	UFUNCTION(BlueprintPure)
	bool UsingPowerup();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AEMG_SlipperyHazzard> SlipperyHazardRef;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TSubclassOf<AEMG_ObstacleHazard> GooeyHazardRef;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Replicated)
	FTimerHandle PowerupTimerHandle;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Replicated)
	int CurrentPowerupDuration;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	float PowerupRemainingTime;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Replicated)
	float PowerupPercent;

	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	UPROPERTY(BlueprintReadOnly)
	AEMGBasePlayer* PlayerRef;

	
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Powerup Duration")
	float SpeedPowerupDuration = 10;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Powerup Duration")
	float InventoryPowerupDuration = 20;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Powerup Duration")
	float ShieldPowerupDuration = 10;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float InventoryPowerUpCap = 100;
	
	UFUNCTION()
	EPowerUpType GetRandomPowerup();

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_AssignPlayerPowerup();
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_AssignPlayerPowerup(EPowerUpType PowerupToAssign);
	
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_UsePowerup();

	
	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_RevertPowerup();
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_RevertPowerup();

	UFUNCTION(Server, Reliable)
	void Server_SpeedPowerup();	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpeedPowerup();

	UFUNCTION(Server,Reliable)
	void Server_UseShieldPowerup();
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_UseShieldPowerup();
	

	UFUNCTION()
	void RevertSpeedPowerup();

	UFUNCTION()
	void InfiniteInventory();

	UFUNCTION()
	void RevertInfiniteInventory();

	UFUNCTION()
	int GetPlayerScoreAverage();

	UFUNCTION(Server,Reliable)
	void Server_UpdateRemainingPowerupTime();

	UFUNCTION(Client,Reliable)
	void Client_UpdateRemainingPowerupTime(float remainingTime);
	
	FActorSpawnParameters PowerupSpawnParams;

	USceneComponent* PowerupSpawn;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	EPowerUpType CurrentPowerup = EPowerUpType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPowerUpSettings PowerUpSettings;
	
	

};
