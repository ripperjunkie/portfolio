// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "EMGUtils.h"
#include "Components/ActorComponent.h"
#include "EMGStealAbility.generated.h"


class UEMGPlayerInventory;
class UEMGRamAbility;
class AEMGBasePlayer;
class AShoppingItem;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EVERYTHINGMUSTGO_API UEMGStealAbility : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEMGStealAbility();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Assign this component ref inside the player */
	UPROPERTY(BlueprintReadWrite, Category=References)
	UEMGRamAbility* RamAbilityRef;

	UPROPERTY(BlueprintReadWrite, Category=References)
	AEMGBasePlayer* BasePlayerRef;

	UPROPERTY()
	TArray<FItemsInInventory> ItemsToSteal;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Item spread")
	int ItemSpreadRadius;

	// Function to handle collision
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void QueryHitActor(const AActor* OtherActor);

	/* Minimum velocity to steal item*/
	UPROPERTY(EditAnywhere, Category=Ability)
	float StealMinVelocity = 1000.f;

	
	
protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AShoppingItem> BPShoppingItemRef;
	
	UFUNCTION()
	void StealDivisionOfInventory(UEMGPlayerInventory* HitPlayerInventory,float DivideInventoryBy);
	
	UFUNCTION(Server, Reliable)
	void Server_StealItem(UEMGPlayerInventory* HitPlayerInventory);

	UFUNCTION(NetMulticast, Reliable)	
	void Multicast_StealItem(UEMGPlayerInventory* HitPlayerInventory);

	UFUNCTION(Server, Reliable)
	void Server_SwapItem(UEMGPlayerInventory* HitPlayerInventory);

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_SpawnStolenItems(FItemsInInventory ItemToSpawn);
	
};
