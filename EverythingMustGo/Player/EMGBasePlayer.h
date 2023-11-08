// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "EMGGameMode.h"
#include "GameFramework/Character.h"
#include "Interfaces/EMGPlayerInterface.h"


#include "EMGBasePlayer.generated.h"


class UGameplayStatics;
class USpringArmComponent;
class UCameraComponent;
class UEMGPlayerInventory;
class AEMGPlayerCamera;
class UEMGRamAbility;
class UEMGStealAbility;
class UEMGPowerup;

 UENUM(BlueprintType)
 enum class EActionState : uint8
 {
 	None = 0,
 	Movement = 1 << 0,
 	Ramming = 1 << 2,
 	CheckingOut = 1 << 3,
 	ChargingRam = 1 << 4,
 	RechargingRam = 1 << 5,
 	StillCheckingOut = 1 << 6
 };


UCLASS()
class EVERYTHINGMUSTGO_API AEMGBasePlayer : public ACharacter, public IEMGPlayerInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEMGBasePlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UEMGPlayerInventory* InventoryComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UEMGRamAbility* RamAbilityComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UEMGStealAbility* StealAbilityComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
	UEMGPowerup* PowerupComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Component)
	USceneComponent* PowerupSpawn;

	UPROPERTY()
	AEMGGameMode* GameModeRef;

	UPROPERTY()
	FRotator InitialRotationRate;
	
	UPROPERTY()
	float InitialMaxAcceleration;

	UPROPERTY()
	float InitialGroundFriction;

	UPROPERTY()
	float InitialMaxSpeed;

	UPROPERTY()
	float InitialMass;

	UPROPERTY()
	FVector PlayerNormalVector;

	UPROPERTY()
	FVector PlayerStartVector;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float MinimumHitAngle = 40.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float MaxHitAngle = 140.0f;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	bool bCanTriggerPowerUp;

	UFUNCTION(BlueprintPure)
	bool HasMatchStarted() const;

	virtual void AddShoppingItemToInventory_Implementation(FItemsInInventory itemToAdd) override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SetCurrentState(EActionState state);	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_SetCurrentState(EActionState state);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ResetState(EActionState state);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_ResetState(EActionState state);

	UFUNCTION(BlueprintImplementableEvent)
	void ResetPowewrupWidget();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ActivateRammingParticles();
	
	UFUNCTION(BlueprintPure, BlueprintCallable)
	FORCEINLINE bool CheckState(EActionState stateToCheck)
	{
		// check if that particular bit is on or not
		return (CurrentActionState & static_cast<int>(stateToCheck)) > (static_cast<int>(stateToCheck) - 1);
	}

	/* Inherited interface methods */
	bool HasItems_Implementation();	
	void StartedCheckingOut_Implementation();
	void CompletedCheckingOut_Implementation();
	void InterruptedCheckingOut_Implementation();
	void OnGettingRammed_Implementation();
	
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void MoveForward_Input(float AxisValue);
	void MoveRight_Input(float AxisValue);
	
	void Ram_Input();
	void Interact_Input();

	/* VEHICLE CONTROL SCHEME (EXPERIMENTAL) */
	void Accelerate_Input(float AxisValue);
	void Brake_Input(float AxisValue);
	void Turn_Input(float AxisValue);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle Control Scheme")
	float TurnRotationSpeed = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle Control Scheme")
	bool bShouldUseVehicleControlScheme;
	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	int CurrentActionState;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	UDataTable* LowValueItemsDT;

	UPROPERTY(EditDefaultsOnly, Category="Debug")
	UDataTable* MidValueItemsDT;
	
	UPROPERTY(EditDefaultsOnly, Category="Debug")
	UDataTable* HighValueItemsDT;


};
