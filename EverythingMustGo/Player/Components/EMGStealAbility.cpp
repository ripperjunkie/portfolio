// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin

#include "EMGStealAbility.h"

#include "DrawDebugHelpers.h"
#include "EMGPlayerInventory.h"
//#include "../../../../../../../../../../../Program Files (x86)/Windows Kits/10/Include/10.0.19041.0/um/Audioclient.h"
#include "EMGPowerup.h"
#include "Components/CapsuleComponent.h"
#include "EverythingMustGo/Player/EMGBasePlayer.h"
#include "EverythingMustGo/Player/Components/EMGRamAbility.h"


#define PRINT(x) GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, TEXT(x))


// Sets default values for this component's properties
UEMGStealAbility::UEMGStealAbility()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UEMGStealAbility::BeginPlay()
{
	Super::BeginPlay();

	if(!GetOwner())
		return;

	// Assign player reference
	BasePlayerRef = Cast<AEMGBasePlayer>(GetOwner());

	// Bind steal ability event to on hit from character collision
	BasePlayerRef->GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &UEMGStealAbility::OnHit);
}

void UEMGStealAbility::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Bind steal ability event to on hit from character collision
	BasePlayerRef->GetCapsuleComponent()->OnComponentHit.RemoveDynamic(this, &UEMGStealAbility::OnHit);
}


// Called every frame
void UEMGStealAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UEMGStealAbility::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if(!RamAbilityRef)
		return;

	AEMGBasePlayer* OtherPlayerRef = Cast<AEMGBasePlayer>( OtherActor);

	if(!OtherPlayerRef) {return;}
	

	if(OtherPlayerRef->PowerupComponent->CurrentPowerup == EPowerUpType::Shield)
	{
		return;
	}
	
	// Check if player is using ram ability
	if(RamAbilityRef->GetUsingAbility())
	{
		if(OtherActor)
		{
			QueryHitActor(OtherActor);
		}
	}
}

void UEMGStealAbility::QueryHitActor(const AActor* OtherActor)
{	
	if(!IsValid(OtherActor))
	{
		//UE_LOG(LogTemp,Warning,TEXT("Invalid hit actor"));
		return;
	}
	
	
	// TODO: Check if player has enough velocity
	if(BasePlayerRef->GetVelocity().Size() < StealMinVelocity)
	{
		//UE_LOG(LogTemp,Warning,TEXT("Not enough speed to steal item"));
		return;
	}

	// reset ability if hit opponent, so it prevents calling this function multiple times
	RamAbilityRef->Server_ResetAbility();
	
	// TODO: Check which side player collided with opponent
	const FVector playerVector = BasePlayerRef->GetActorForwardVector().GetSafeNormal(); // gets normalized vectors
	const FVector opponentVector = OtherActor->GetActorForwardVector().GetSafeNormal(); // gets normalized vectors
	
	Server_StealItem( OtherActor->FindComponentByClass<UEMGPlayerInventory>() );
	
//======================================================= Angle hit checck =========================================================================
	
	//const float hitDirection = (180.f)/PI * FMath::Acos( FVector::DotProduct(playerVector, opponentVector) / (opponentVector.Size() * playerVector.Size()) );

	// Steal item if opponent has an item in fact
	// Hit opponent on back
	/*if(hitDirection <= 15.f)
	{		
		Server_SwapItem( OtherActor->FindComponentByClass<UEMGPlayerInventory>() );
		return;
	}*/
	
	// Hit opponent side
	//if(hitDirection >= BasePlayerRef->MinimumHitAngle && hitDirection <= BasePlayerRef->MaxHitAngle)
	//{
		//Server_StealItem( OtherActor->FindComponentByClass<UEMGPlayerInventory>() );
	//}
}

void UEMGStealAbility::StealDivisionOfInventory(UEMGPlayerInventory* HitPlayerInventory,float DivideInventoryBy)
{
	int32 NumItemsToSteal = FMath::CeilToInt(HitPlayerInventory->PlayerInventory.Num()/DivideInventoryBy);
	NumItemsToSteal = FMath::Min(NumItemsToSteal,HitPlayerInventory->PlayerInventory.Num());
		
	//for(FItemsInInventory items : HitPlayerInventory->PlayerInventory)
	for(int32 i=0 ; i < NumItemsToSteal; i++)
	{
		ItemsToSteal.Add(HitPlayerInventory->PlayerInventory[i]);
		HitPlayerInventory->Server_RemoveLastItem();
	}
}



void UEMGStealAbility::Server_SwapItem_Implementation(UEMGPlayerInventory* HitPlayerInventory)
{
	// TODO: Swap item when opponents are colliding in opposite ways
}

void UEMGStealAbility::Server_StealItem_Implementation(UEMGPlayerInventory* HitPlayerInventory)
{
	//Multicast_StealItem(HitPlayerInventory);
	if(!HitPlayerInventory)
	{
		//	UE_LOG(LogTemp, Display, TEXT("Invalid player inventory"));
		return;
	}
	if(HitPlayerInventory->PlayerInventory.Num() == 0)
	{
		//UE_LOG(LogTemp, Display, TEXT("No items to steal"));		
		return;
	}

	//TODO: Check for the tier of the hit to determine how many items are being stolen

	if(BasePlayerRef->RamAbilityComp->ChargeBarMultiplier >= 0.75)
	{
		//Grabs half of the opponent's inventory and assigns it to ItemsToSteal
		StealDivisionOfInventory(HitPlayerInventory,2);

		//Put ItemsToSteal into the player's inventory (comented for spawning items on floor)
		//BasePlayerRef->InventoryComp->Server_AddMultipleItemsToInventory(ItemsToSteal);
		
		//TODO: Spawn items in floor implementation
		for(FItemsInInventory item :ItemsToSteal)
		{
			/*AShoppingItem* StolenItem;
			StolenItem->Item = item;*/
			Server_SpawnStolenItems(item);
		}
		
		//Clears Items To Steal
		ItemsToSteal.Empty();
	}

	else if(BasePlayerRef->RamAbilityComp->ChargeBarMultiplier >= 0.40 && BasePlayerRef->RamAbilityComp->ChargeBarMultiplier <= 0.74)
	{
		//Grabs 1/4 of the opponent's inventory and assigns it it to ItemsToSteal
		StealDivisionOfInventory(HitPlayerInventory,4);

		//Adds items in ItemsToSteal to the players inventory (comented for spawning items on floor)
		//BasePlayerRef->InventoryComp->Server_AddMultipleItemsToInventory(ItemsToSteal);

		for(FItemsInInventory item :ItemsToSteal)
		{
			/*AShoppingItem StolenItem;
			StolenItem.Item = item;*/
			Server_SpawnStolenItems(item);
		}

		//Clears Items To Steal
		ItemsToSteal.Empty();
	}

	else
	{
		FItemsInInventory LastItem = HitPlayerInventory->PlayerInventory.Last();
		if(LastItem.ItemSize <= BasePlayerRef->InventoryComp->ItemSpace)
		{
			FItemsInInventory StealingItem = HitPlayerInventory->PlayerInventory.Last();
			HitPlayerInventory->Server_RemoveLastItem();
			BasePlayerRef->InventoryComp->Server_AddItemToInventory(StealingItem);		
		}
	}
}

void UEMGStealAbility::Multicast_StealItem_Implementation(UEMGPlayerInventory* HitPlayerInventory)
{
	if(!HitPlayerInventory)
	{
		return;
	}
	if(HitPlayerInventory->PlayerInventory.Num() == 0)
	{
		return;
	}

	ItemSpreadRadius *= BasePlayerRef->RamAbilityComp->ChargeBarMultiplier;

	if(BasePlayerRef->RamAbilityComp->ChargeBarMultiplier >= 0.75)
	{
		//Grabs half of the opponent's inventory and assigns it to ItemsToSteal
		StealDivisionOfInventory(HitPlayerInventory,2);

		//Put ItemsToSteal into the player's inventory (comented for spawning items on floor)
		//BasePlayerRef->InventoryComp->Server_AddMultipleItemsToInventory(ItemsToSteal);
		
		for(FItemsInInventory item :ItemsToSteal)
		{
			Server_SpawnStolenItems(item);
		}
		
		//Clears Items To Steal
		ItemsToSteal.Empty();
	}

	else if(BasePlayerRef->RamAbilityComp->ChargeBarMultiplier >= 0.40 && BasePlayerRef->RamAbilityComp->ChargeBarMultiplier <= 0.74)
	{
		//Grabs 1/4 of the opponent's inventory and assigns it it to ItemsToSteal
		StealDivisionOfInventory(HitPlayerInventory,4);

		//Adds items in ItemsToSteal to the players inventory (comented for spawning items on floor)
		//BasePlayerRef->InventoryComp->Server_AddMultipleItemsToInventory(ItemsToSteal);

		for(FItemsInInventory item :ItemsToSteal)
		{
			Server_SpawnStolenItems(item);
		}

		//Clears Items To Steal
		ItemsToSteal.Empty();
	}

	else
	{
		FItemsInInventory LastItem = HitPlayerInventory->PlayerInventory.Last();
		if(LastItem.ItemSize <= BasePlayerRef->InventoryComp->ItemSpace)
		{
			FItemsInInventory StealingItem = HitPlayerInventory->PlayerInventory.Last();
			HitPlayerInventory->Server_RemoveLastItem();
			BasePlayerRef->InventoryComp->Server_AddItemToInventory(StealingItem);		
		}
	}
	
}

void UEMGStealAbility::Server_SpawnStolenItems_Implementation(FItemsInInventory ItemToSpawn)
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//Scatter products on a circle around the player that lost them
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	float Angle = FMath::FRand() * 2.0f * PI;
	float Distance = FMath::Sqrt(FMath::FRand()) * ItemSpreadRadius;

	FVector CircleVector = FVector(Distance * FMath::Cos(Angle),Distance * FMath::Sin(Angle), 0.0f);
	FVector SpawnLocation = PlayerLocation + CircleVector;
	
	AShoppingItem* NewItem = GetWorld()->SpawnActor<AShoppingItem>(BPShoppingItemRef,
		SpawnLocation,GetOwner()->GetActorRotation(), SpawnParams);

	if(NewItem)
	{
		NewItem->Item = ItemToSpawn;
		NewItem->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(ItemToSpawn.ItemMesh);
		NewItem->SetParticle(ItemToSpawn.NiagaraParticles);
	}
}
