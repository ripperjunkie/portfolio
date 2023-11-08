// Copyright (c) 2023 @ Runaway Concepts

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EMGUtils.h"

#include "EMGPlayerInventory.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddInventoryItem, FItemsInInventory, ItemAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveInventoryItem, FItemsInInventory, ItemRemoved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCheckOut);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class EVERYTHINGMUSTGO_API UEMGPlayerInventory : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEMGPlayerInventory();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float InventoryCapacity = 10;

	UPROPERTY()
	float InitialInventoryCapacity;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float ItemCount;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float ItemSpace;

	UPROPERTY()
	bool bIsWeightAffectingMovement = true;

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<FItemsInInventory> PlayerInventory;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float TotalValueCollected;

	UPROPERTY(BlueprintReadOnly)
	ACharacter* PlayerRef;

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_AddItemToInventory(FItemsInInventory ItemToAdd);
	
	UFUNCTION(NetMulticast, Reliable,BlueprintCallable)
	void Multicast_AddToInventory(FItemsInInventory ItemsInInventory);

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_AddMultipleItemsToInventory(const TArray<FItemsInInventory>& ItemsToAdd);

	UFUNCTION(NetMulticast, Reliable,BlueprintCallable)
	void Multicast_AddMultipleItemsToInventory(const TArray<FItemsInInventory>& ItemsToAdd);

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_RemoveLastItem();
	
	UFUNCTION(NetMulticast,Reliable,BlueprintCallable)
	void MulticastRemoveLastItem();

	UFUNCTION(Server,Reliable,BlueprintCallable)
	void Server_RemoveFirstItem();
	
	UFUNCTION(NetMulticast,Reliable,BlueprintCallable)
	void MulticastRemoveFirstItem();

	UFUNCTION()
	void UpdateTotalValueCollected();


	UFUNCTION(Server,Reliable, BlueprintCallable)
	void Server_EmptyInventory();

	UFUNCTION(NetMulticast,Reliable)
	void Multicast_EmptyInventory();

	UFUNCTION()
	void UpdatePlayersCartWeight();

	UPROPERTY(BlueprintAssignable)
	FOnAddInventoryItem OnAddItemToInventory;
	UPROPERTY(BlueprintAssignable)
	FOnRemoveInventoryItem OnRemoveInventoryItem;
	UPROPERTY(BlueprintAssignable)
	FOnCheckOut OnCheckOut;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Item Slowdown Multipliers")
	float MaxAccelerationMutliplier;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Item Slowdown Multipliers")
	float MaxSpeedMultiplier;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Item Slowdown Multipliers")
	float MassMultiplier;

	UPROPERTY(EditAnywhere,BlueprintReadOnly, Category="Item Slowdown Multipliers")
	float FrictionMultiplier;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
