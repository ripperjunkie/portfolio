// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin


#include "EMGPlayerInventory.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "EMGUtils.h"
#include "EverythingMustGo/Player/EMGBasePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"


UEMGPlayerInventory::UEMGPlayerInventory()
{
	PrimaryComponentTick.bCanEverTick = true;
}



void UEMGPlayerInventory::BeginPlay()
{
	Super::BeginPlay();

	PlayerRef = Cast<ACharacter>(GetOwner());
	ItemSpace = InventoryCapacity;
	InitialInventoryCapacity = InventoryCapacity;
	
}



void UEMGPlayerInventory::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

//Adds new collected item to the players inventory
void UEMGPlayerInventory::Multicast_AddToInventory_Implementation(FItemsInInventory ItemsInInventory)
{
	//Add the new item as a copy
	ItemCount += ItemsInInventory.ItemSize;
	ItemSpace -= ItemsInInventory.ItemSize;
	
	PlayerInventory.Add(ItemsInInventory);
	UpdateTotalValueCollected();
	OnAddItemToInventory.Broadcast(ItemsInInventory);

	//TODO: Implement logic for affecting the players movement
	UpdatePlayersCartWeight();
}

void UEMGPlayerInventory::Multicast_AddMultipleItemsToInventory_Implementation(const TArray<FItemsInInventory>& ItemsToAdd)
{
	for(FItemsInInventory items:ItemsToAdd)
	{
		PlayerInventory.Add(items);
		ItemCount += items.ItemSize;
		ItemSpace -= items.ItemSize;
		OnAddItemToInventory.Broadcast(items);
	}

	UpdateTotalValueCollected();

	//TODO: Add function for adding weight to the player's cart
}

void UEMGPlayerInventory::Server_AddItemToInventory_Implementation(FItemsInInventory ItemToAdd)
{
	Multicast_AddToInventory(ItemToAdd);
}

void UEMGPlayerInventory::Server_AddMultipleItemsToInventory_Implementation(const TArray<FItemsInInventory>& ItemsToAdd)
{
	Multicast_AddMultipleItemsToInventory(ItemsToAdd);
}

void UEMGPlayerInventory::Server_RemoveLastItem_Implementation()
{
	MulticastRemoveLastItem();
}


void UEMGPlayerInventory::MulticastRemoveLastItem_Implementation()
{
	if(PlayerInventory.Num() > 0)
	{
		FItemsInInventory LastItem = PlayerInventory.Last();
		OnRemoveInventoryItem.Broadcast(LastItem);
		ItemCount -= LastItem.ItemSize;
		ItemSpace += LastItem.ItemSize;
		PlayerInventory.Pop();
	}

	UpdateTotalValueCollected();

	//TODO: Add logic to reset the players movement
	UpdatePlayersCartWeight();
}

void UEMGPlayerInventory::Server_RemoveFirstItem_Implementation()
{
	MulticastRemoveFirstItem();
}

void UEMGPlayerInventory::MulticastRemoveFirstItem_Implementation()
{
	if(PlayerInventory.Num() > 0)
	{
		FItemsInInventory FirstItem = PlayerInventory.Top();
		OnRemoveInventoryItem.Broadcast(FirstItem);
		ItemCount -= FirstItem.ItemSize;
		ItemSpace += FirstItem.ItemSize;
		PlayerInventory.Pop();
	}

	UpdateTotalValueCollected();

	//TODO: Add logic to reset the players movement
	UpdatePlayersCartWeight();
}




//Sums the values of all the items currently on inventory
void UEMGPlayerInventory::UpdateTotalValueCollected()
{
	//Restart the counter before updating
	TotalValueCollected = 0;

	for(const FItemsInInventory items:PlayerInventory)
	{
		TotalValueCollected += items.Value;
		
	}
}


//updates score on checkout (server side)
void UEMGPlayerInventory::Server_EmptyInventory_Implementation()
{
	Multicast_EmptyInventory();
}

void UEMGPlayerInventory::Multicast_EmptyInventory_Implementation()
{
	PlayerInventory.Empty();
	TotalValueCollected = 0;
	ItemCount = 0;
	ItemSpace = InventoryCapacity;
	OnCheckOut.Broadcast();

	//TODO: Add logic to reset the players movement
	UpdatePlayersCartWeight();
}

void UEMGPlayerInventory::UpdatePlayersCartWeight()
{
	if(bIsWeightAffectingMovement)
	{
		AEMGBasePlayer* player = Cast<AEMGBasePlayer>(PlayerRef);
		
		if(!player)
		{
			return;
		}

		if(ItemCount >= InventoryCapacity)
		{
			player->GetCharacterMovement()->MaxAcceleration = player->InitialMaxAcceleration - (MaxAccelerationMutliplier * InventoryCapacity);
			player->GetCharacterMovement()->GroundFriction = player->InitialGroundFriction - (FrictionMultiplier* InventoryCapacity);
			player->GetCharacterMovement()->Mass = player->InitialMass + (MassMultiplier * InventoryCapacity);
			player->GetCharacterMovement()->MaxWalkSpeed = player->InitialMaxSpeed - (MaxSpeedMultiplier * InventoryCapacity);
		}

		else
		{
			player->GetCharacterMovement()->MaxAcceleration = player->InitialMaxAcceleration - (MaxAccelerationMutliplier * ItemCount);
			player->GetCharacterMovement()->GroundFriction = player->InitialGroundFriction - (FrictionMultiplier* ItemCount);
			player->GetCharacterMovement()->Mass = player->InitialMass + (MassMultiplier * ItemCount);
			player->GetCharacterMovement()->MaxWalkSpeed = player->InitialMaxSpeed - (MaxSpeedMultiplier * ItemCount);
		}
		
	}
	
}
