// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin

#include "EMGPowerup.h"

#include "EMGPlayerInventory.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UEMGPowerup::UEMGPowerup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...


}

// Called when the game starts
void UEMGPowerup::BeginPlay()
{                                                                  
	Super::BeginPlay();

	PlayerRef = Cast<AEMGBasePlayer>(GetOwner());
	PowerupSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	PowerupSpawn = GetOwner()->FindComponentByClass<USceneComponent>();
}

void UEMGPowerup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	
}

// Called every frame
void UEMGPowerup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(CurrentPowerupDuration > 0)
	{
		Server_UpdateRemainingPowerupTime();
	}
}




void UEMGPowerup::Server_UsePowerup_Implementation()
{
	
	switch (CurrentPowerup)
	{
		
	case EPowerUpType::SpeedBoost:
		{
			Server_SpeedPowerup();
			GetWorld()->GetTimerManager().SetTimer(PowerupTimerHandle,this, &UEMGPowerup::Server_RevertPowerup,SpeedPowerupDuration,false);
			
			break;
		}
			
	case EPowerUpType::InfiniteInventory:
		{
			InfiniteInventory();
			//CurrentPowerupDuration = InventoryPowerupDuration;
			GetWorld()->GetTimerManager().SetTimer(PowerupTimerHandle,this, &UEMGPowerup::Server_RevertPowerup,InventoryPowerupDuration,false);
			break;
		}

	case EPowerUpType::Shield:
		{
			Server_UseShieldPowerup();
			GetWorld()->GetTimerManager().SetTimer(PowerupTimerHandle,this, &UEMGPowerup::Server_RevertPowerup,ShieldPowerupDuration,false);
			break;
		}


	case EPowerUpType::Gooey:
		{
			if(!GooeyHazardRef)
			{
				return;
			}
			if(!PowerupSpawn)
			{
				return;
			}
			
			FVector spawningLocation = PowerupSpawn->GetComponentLocation();
			AEMG_ObstacleHazard* GooeyHazard = GetWorld()->SpawnActor<AEMG_ObstacleHazard>(GooeyHazardRef,spawningLocation,
				GetOwner()->GetActorRotation(),PowerupSpawnParams);
		
			Server_RevertPowerup();
			break;
		}

	case EPowerUpType::Slippery:
		{
			if(!SlipperyHazardRef)
			{
				return;
			}
			if(!PowerupSpawn)
			{
				return;
			}
			
			FVector spawningLocation = PowerupSpawn->GetComponentLocation();
			AEMG_SlipperyHazzard* slipperyHazard = GetWorld()->SpawnActor<AEMG_SlipperyHazzard>(SlipperyHazardRef,spawningLocation,
				GetOwner()->GetActorRotation(),PowerupSpawnParams);
		
			Server_RevertPowerup();
			break;
		}
		
	}
}






void UEMGPowerup::Server_RevertPowerup_Implementation()
{
	Multicast_RevertPowerup();
}

void UEMGPowerup::Multicast_RevertPowerup_Implementation()
{
	switch (CurrentPowerup)
	{
		case EPowerUpType::SpeedBoost:
			RevertSpeedPowerup();
			break;
		
		case EPowerUpType::InfiniteInventory:
			RevertInfiniteInventory();
			break;
	}
	
	CurrentPowerup = EPowerUpType::None;
	PlayerRef->ResetPowewrupWidget();
	CurrentPowerupDuration = 0;
}

EPowerUpType UEMGPowerup::GetRandomPowerup()
{

	//return EPowerUpType::Shield;
	//Get score average in match
	float avgScore = GetPlayerScoreAverage();

	//tiers probability of powerup per tier (t1 is OP, tier3 is lame)
	float tier1 = 0.25;
	float tier2 = 0.35;
	float tier3 = 0.40;

	if(!PlayerRef)
	{
		return EPowerUpType::None;
	}
	
	if(PlayerRef->GetPlayerState()->GetScore() < avgScore * 0.5)
	{
		tier1 += 0.35;
		tier2 -= 0.10;
		tier3 -= 0.25;
	}

	if(PlayerRef->GetPlayerState()->GetScore() > avgScore * 0.5)
	{
		tier1 -= 0.1;
		tier2 -= 0.1;
		tier3 += 0.2;
	}

	float RandomTierValue = FMath::FRand();

	if(RandomTierValue < tier1)
	{
		//assign powerup between InfiniteInventory and Shield	
		int32 RandomValue = FMath::RandRange(static_cast<int32>(EPowerUpType::InfiniteInventory),static_cast<int32>(EPowerUpType::Shield));
		return static_cast<EPowerUpType>(RandomValue);
	}

	if(RandomTierValue < tier1 + tier2)
	{
		//assign SpeedBoost
		return EPowerUpType::SpeedBoost;
	}

	else
	{
		//assign between Slippery and Gooey
		int32 RandomValue = FMath::RandRange(static_cast<int32>(EPowerUpType::Slippery),static_cast<int32>(EPowerUpType::Gooey));
		return static_cast<EPowerUpType>(RandomValue);
	}
	
}

void UEMGPowerup::Server_AssignPlayerPowerup_Implementation()
{
	EPowerUpType AssignedPowerup = GetRandomPowerup();
	Multicast_AssignPlayerPowerup(AssignedPowerup);
}

void UEMGPowerup::Multicast_AssignPlayerPowerup_Implementation(EPowerUpType PowerupToAssign)
{
	CurrentPowerup = PowerupToAssign;
}

void UEMGPowerup::Server_SpeedPowerup_Implementation()
{
	Multicast_SpeedPowerup();
}

void UEMGPowerup::Multicast_SpeedPowerup_Implementation()
{
	if(!PlayerRef)
	{
		return;
	}

	CurrentPowerupDuration = SpeedPowerupDuration;
	
	PlayerRef->InventoryComp->bIsWeightAffectingMovement = false;
	
	PlayerRef->GetCharacterMovement()->MaxWalkSpeed = PowerUpSettings.Speed;
	PlayerRef->GetCharacterMovement()->MaxAcceleration = PowerUpSettings.Acceleration;
	
	const FRotator PlayerRotation = PlayerRef->GetCharacterMovement()->RotationRate; 
	PlayerRef->GetCharacterMovement()->RotationRate = FRotator(PlayerRotation.Pitch,PowerUpSettings.RotationRate,PlayerRotation.Roll);	
}


void UEMGPowerup::Server_UseShieldPowerup_Implementation()
{
	Multicast_UseShieldPowerup();
}

void UEMGPowerup::Multicast_UseShieldPowerup_Implementation()
{
	CurrentPowerupDuration = ShieldPowerupDuration;
}

void UEMGPowerup::RevertSpeedPowerup()
{
	PlayerRef->InventoryComp->bIsWeightAffectingMovement = true;
	
	PlayerRef->GetCharacterMovement()->MaxWalkSpeed = PlayerRef->InitialMaxSpeed;
	PlayerRef->GetCharacterMovement()->RotationRate = PlayerRef->InitialRotationRate;
	PlayerRef->GetCharacterMovement()->MaxAcceleration = PlayerRef->InitialMaxAcceleration;
}

void UEMGPowerup::InfiniteInventory()
{
	PlayerRef->InventoryComp->bIsWeightAffectingMovement = false;

	CurrentPowerupDuration = InventoryPowerupDuration;
	
	PlayerRef->GetCharacterMovement()->MaxAcceleration = PlayerRef->InitialMaxAcceleration;
	PlayerRef->GetCharacterMovement()->MaxWalkSpeed = PlayerRef->InitialMaxSpeed;
	PlayerRef->GetCharacterMovement()->GroundFriction = PlayerRef->InitialGroundFriction;
	PlayerRef->GetCharacterMovement()->Mass = PlayerRef->InitialMass;
	
	
	PlayerRef->InventoryComp->InventoryCapacity = InventoryPowerUpCap;
}

void UEMGPowerup::RevertInfiniteInventory()
{
	PlayerRef->InventoryComp->InventoryCapacity = PlayerRef->InventoryComp->InitialInventoryCapacity;

	PlayerRef->InventoryComp->bIsWeightAffectingMovement = true;
	PlayerRef->InventoryComp->UpdatePlayersCartWeight();
}


bool UEMGPowerup::UsingPowerup()                                                                    
{                                                                                                   
	check(GetWorld())	                                                                            
	return GetWorld()->GetTimerManager().GetTimerRemaining(PowerupTimerHandle) > 0.f;               
}                                                                                                   
                                                                                                    
int UEMGPowerup::GetPlayerScoreAverage()
{
	int ScoreSum = 0;
	int ScoreAverage = 0;
	
	TArray<APlayerState*> PlayerStates = GetWorld()->GetGameState()->PlayerArray;
	
	if(PlayerStates.Num() == 0)
	{
		return 0;
	}

	for (auto PlayerState : PlayerStates)
	{
		ScoreSum += PlayerState->GetScore();
	}

	ScoreAverage = ScoreSum/PlayerStates.Num();

	return ScoreAverage;
}

void UEMGPowerup::Server_UpdateRemainingPowerupTime_Implementation()
{
	float remainingTime = GetWorld()->GetTimerManager().GetTimerRemaining(PowerupTimerHandle);
	PowerupPercent = remainingTime/CurrentPowerupDuration;
	Client_UpdateRemainingPowerupTime(PowerupPercent);
}

void UEMGPowerup::Client_UpdateRemainingPowerupTime_Implementation(float remainingTime)
{
	PowerupPercent = remainingTime;
}

