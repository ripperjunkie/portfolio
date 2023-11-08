// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Components/PrimitiveComponent.h"


#include "EMGRamAbility.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRamAbility, Display, All);

class AEMGBasePlayer;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EVERYTHINGMUSTGO_API UEMGRamAbility : public UActorComponent
{
	GENERATED_BODY()

public:
	UEMGRamAbility();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeBarMultiplier;

	UFUNCTION(BlueprintCallable)
	void UseAbility();

	UFUNCTION(Server, Reliable)
	void Server_SetData();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetData();

	UFUNCTION(Server, Reliable)
	void Server_ResetAbility();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetAbility();

	UFUNCTION(BlueprintCallable,Server,Reliable)
	void Server_SetMultipliers(float NewMultiplier);
	UFUNCTION(NetMulticast,Reliable)
	void Multicast_SetMultipliers(float NewMultiplier);

	/* Knock back other target on ram */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_KnockbackTarget(AEMGBasePlayer* Target, FVector Direction);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_KnockbackTarget(AEMGBasePlayer* Target, FVector Direction);

	UFUNCTION(BlueprintPure)
	bool GetCanUseAbility() const
	{
		return bCanUseAbility;
	}

	UFUNCTION(BlueprintPure)
	bool GetUsingAbility() const
	{
		return bUsingAbility;
	}

	UFUNCTION(Server,Reliable, BlueprintCallable)
	void Server_CooldownRamming();

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_CooldownRamming();


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// Function to handle collision
	UFUNCTION(Server, Reliable)
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	
	
	UPROPERTY(BlueprintReadWrite)
	bool bCanUseAbility = true;

	UPROPERTY(BlueprintReadWrite)
	bool bUsingAbility = false;


	UPROPERTY(BlueprintReadOnly)
	FTimerHandle AbilityTH;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CooldownTimer = 3.f;

	UPROPERTY(BlueprintReadOnly)
	AEMGBasePlayer* PlayerRef;

	/* Speed for when ramming */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NewSpeed = 1200.f;

	/* Acceleration for when ramming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NewMaxAcceleration = 2048.f;
	
	/* Knockback force for when ramming hit opponent */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float KnockbackForce = 150.f;

	UPROPERTY()
	float InitialMaxAcceleration;

	UPROPERTY()
	float InitialSpeed;

private:
	UPROPERTY()
	float PlayerDefaultSpeed;
	UPROPERTY()
	float DefaultMaxAcceleration;
};
