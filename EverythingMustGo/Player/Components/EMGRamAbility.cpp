// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin

#include "EMGRamAbility.h"

#include "EMGPowerup.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "EverythingMustGo/Player/EMGBasePlayer.h"
#include "EverythingMustGo/Player/Interfaces/EMGPlayerInterface.h"
#include "GameFramework/CharacterMovementComponent.h"

#define PRINT(time, text) if(GEngine) \
GEngine->AddOnScreenDebugMessage(-1, time, FColor::Green, FString(text));

DEFINE_LOG_CATEGORY(LogRamAbility);

// Sets default values for this component's properties
UEMGRamAbility::UEMGRamAbility()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void UEMGRamAbility::Server_SetMultipliers_Implementation(float NewMultiplier)
{
	Multicast_SetMultipliers(NewMultiplier);
}

void UEMGRamAbility::Multicast_SetMultipliers_Implementation(float NewMultiplier)
{
	ChargeBarMultiplier = NewMultiplier;
}

void UEMGRamAbility::Server_KnockbackTarget_Implementation(AEMGBasePlayer* Target, FVector Direction)
{
	Multicast_KnockbackTarget(Target, Direction);
}

void UEMGRamAbility::Multicast_KnockbackTarget_Implementation(AEMGBasePlayer* Target, FVector Direction)
{
	if(!Target)
		return;

	Target->LaunchCharacter(Direction.GetSafeNormal() * KnockbackForce * ChargeBarMultiplier, true, false);

	// when the player that is getting hit, we tell it to interrupt checking out
	if(IEMGPlayerInterface* PlayerInterface = Cast<IEMGPlayerInterface>(Target))
	{
		IEMGPlayerInterface::Execute_InterruptedCheckingOut(Target);
	}
}



// Called when the game starts
void UEMGRamAbility::BeginPlay()
{
	Super::BeginPlay();

	// Get player ref
	PlayerRef = Cast<AEMGBasePlayer>(GetOwner());

	// store player default speed
	PlayerDefaultSpeed = PlayerRef->GetCharacterMovement()->MaxWalkSpeed;
	
	// store player max acceleration	
	DefaultMaxAcceleration = PlayerRef->GetCharacterMovement()->MaxAcceleration;

	if(PlayerRef)
		PlayerRef->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UEMGRamAbility::OnHit);
}


// Called every frame
void UEMGRamAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bUsingAbility)
		return;

	const FVector Direction = PlayerRef->GetActorForwardVector();
	PlayerRef->AddMovementInput(Direction, 1.f);
}



void UEMGRamAbility::OnHit_Implementation(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                          FVector NormalImpulse, const FHitResult& Hit)
{
	AEMGBasePlayer* OtherPlayerRef = Cast<AEMGBasePlayer>(OtherActor);

	if(!OtherPlayerRef) { return; }	
	
	if(!OtherActor)
		return;

	if(!PlayerRef)
		return;

	if (!bUsingAbility)
		return;

	
	
	if(AEMGBasePlayer* HitPlayerTarget = Cast<AEMGBasePlayer>(OtherActor))
	{
		if(HitPlayerTarget->PowerupComponent->UsingPowerup() && HitPlayerTarget->PowerupComponent->CurrentPowerup == EPowerUpType::Shield)
		{
			return;
		}

		// Knock back other player	
		FVector KnockbackDir = PlayerRef->GetActorForwardVector();
		Server_KnockbackTarget(HitPlayerTarget, KnockbackDir);
		//HitPlayerTarget->LaunchCharacter(KnockbackDir.GetSafeNormal() * KnockbackForce, true, false);

		if (HitPlayerTarget->GetClass()->ImplementsInterface(UEMGPlayerInterface::StaticClass()))
		{
			IEMGPlayerInterface::Execute_OnGettingRammed(HitPlayerTarget);
			PlayerRef->ActivateRammingParticles();
		}
	}
}


void UEMGRamAbility::UseAbility()
{
	if(PlayerRef->CheckState(EActionState::RechargingRam))
	{
		return;
	}
	Server_SetData();

	// start timer to reset ability usage
	GetWorld()->GetTimerManager().SetTimer(
		AbilityTH,
		this,
		&UEMGRamAbility::Server_ResetAbility,
		CooldownTimer,
		false);
}


/************************  NETWORK  **************************/

void UEMGRamAbility::Server_SetData_Implementation()
{
	Multicast_SetData();
}

void UEMGRamAbility::Multicast_SetData_Implementation()
{
	bCanUseAbility = false;
	bUsingAbility = true;

	InitialSpeed = PlayerRef->GetCharacterMovement()->MaxWalkSpeed;
	InitialMaxAcceleration = PlayerRef->GetCharacterMovement()->MaxAcceleration;
	
	//MaxTier of multiplier
	if(ChargeBarMultiplier >= 0.75)
	{
		// set player movement velocity
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = InitialSpeed * (1.5+ ChargeBarMultiplier);

		// set player max acceleration
		PlayerRef->GetCharacterMovement()->MaxAcceleration = InitialMaxAcceleration * (1.5+ ChargeBarMultiplier);
	}

	else if(ChargeBarMultiplier < 0.75 && ChargeBarMultiplier > 0.3)
	{
		// set player movement velocity
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = InitialSpeed * (1.2 + ChargeBarMultiplier);

		// set player max acceleration
		PlayerRef->GetCharacterMovement()->MaxAcceleration = InitialMaxAcceleration * (1.2+ ChargeBarMultiplier);
	}

	else
	{
		// set player movement velocity
		PlayerRef->GetCharacterMovement()->MaxWalkSpeed = InitialSpeed * (1 + ChargeBarMultiplier);

		// set player max acceleration
		PlayerRef->GetCharacterMovement()->MaxAcceleration = InitialMaxAcceleration * (1 + ChargeBarMultiplier);
	}

	// Set to ramming state
	PlayerRef->Server_SetCurrentState(EActionState::Ramming);
}

void UEMGRamAbility::Server_ResetAbility_Implementation()
{
	Multicast_ResetAbility();
}

void UEMGRamAbility::Multicast_ResetAbility_Implementation()
{
	bCanUseAbility = true;
	bUsingAbility = false;

	// reset player movement velocity
	PlayerRef->GetCharacterMovement()->MaxWalkSpeed = InitialSpeed;

	// reset player acceleration	
	PlayerRef->GetCharacterMovement()->MaxAcceleration = InitialMaxAcceleration;

	
	// Reset to ramming state
	PlayerRef->Server_ResetState(EActionState::Ramming);
	PlayerRef->Server_SetCurrentState(EActionState::RechargingRam);
	GetWorld()->GetTimerManager().SetTimer(
		AbilityTH,
		this,
		&UEMGRamAbility::Server_CooldownRamming,
		CooldownTimer,
		false);
}

void UEMGRamAbility::Server_CooldownRamming_Implementation()
{
	Multicast_CooldownRamming();
}

void UEMGRamAbility::Multicast_CooldownRamming_Implementation()
{
	PlayerRef->Server_ResetState(EActionState::RechargingRam);
	PlayerRef->Server_SetCurrentState(EActionState::Movement);
}
