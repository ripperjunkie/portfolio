// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin


#include "EMGBasePlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/EMGPlayerInventory.h"
#include "Components/EMGRamAbility.h"
#include "Components/EMGStealAbility.h"
#include "Components/EMGPowerup.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

#include <imgui.h>

#include "EMGGameMode.h"
#include "Components/BillboardComponent.h"
#include "EverythingMustGo/EMGConsoleManager.h"
#include "EverythingMustGo/GameStates/EMGGameState.h"
#include "Kismet/GameplayStatics.h"


#define LOG(time, text) if(GEngine) \
GEngine->AddOnScreenDebugMessage(-1, time, FColor::Green, FString(text));


/* GUI related stuff */
static float RotationRateZ_GUI;
static float MaxWalkSpeed_GUI;
static float BrakingDecelerationWalking_GUI;
static float GroundFriction_GUI;
static float BrakingFrictionFactor_GUI;
static float MaxAcceleration_GUI;

#define MAX_SLIDER_GUI 100000.f



// Sets default values
AEMGBasePlayer::AEMGBasePlayer()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Tell character class to not use controller rotation, it should rotate based on movement
	bUseControllerRotationYaw = false;
	
	if(GetCharacterMovement())
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
	
	// Creating spring component	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm Component"));
	if(IsValid(SpringArmComp))
	{
		SpringArmComp->SetupAttachment(RootComponent);
		SpringArmComp->SocketOffset = FVector(0,0,500.f);
		SpringArmComp->bInheritPitch = false;
		SpringArmComp->bInheritRoll = false;
		SpringArmComp->bInheritYaw = false;
	}

	// Creating camera component
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera Component"));
	if(IsValid(CameraComp))
	{
		CameraComp->SetupAttachment(SpringArmComp);
	}

	// Create player inventory component
	InventoryComp = CreateDefaultSubobject<UEMGPlayerInventory>(TEXT("Player Inventory"));

	// Create ram ability component
	RamAbilityComp = CreateDefaultSubobject<UEMGRamAbility>(TEXT("Ram Ability"));

	// Create steal ability component
	StealAbilityComp = CreateDefaultSubobject<UEMGStealAbility>(TEXT("Steal Ability"));

	//Create powerup component
	PowerupComponent = CreateDefaultSubobject<UEMGPowerup>(TEXT("Powerups"));

	//Create billboard component
	PowerupSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("Powerup Spawn"));


}

// Called to bind functionality to input
void AEMGBasePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Bindings for forward and sideways movement
	PlayerInputComponent->BindAxis(TEXT("MoveForward"),this, &AEMGBasePlayer::MoveForward_Input);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"),this, &AEMGBasePlayer::MoveRight_Input);
	PlayerInputComponent->BindAxis(TEXT("Accelerate"),this, &AEMGBasePlayer::Accelerate_Input);
	PlayerInputComponent->BindAxis(TEXT("Brake"),this, &AEMGBasePlayer::Brake_Input);
	PlayerInputComponent->BindAxis(TEXT("Turn"),this, &AEMGBasePlayer::Turn_Input);
	
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &AEMGBasePlayer::Interact_Input);
	PlayerInputComponent->BindAction(TEXT("Ram"), IE_Released, this, &AEMGBasePlayer::Ram_Input);
}

// Called when the game starts or when spawned
void AEMGBasePlayer::BeginPlay()
{
	Super::BeginPlay();
	
	RotationRateZ_GUI = GetCharacterMovement()->RotationRate.Yaw;
	MaxWalkSpeed_GUI = GetCharacterMovement()->MaxWalkSpeed;
	BrakingDecelerationWalking_GUI = GetCharacterMovement()->BrakingDecelerationWalking;
	GroundFriction_GUI = GetCharacterMovement()->GroundFriction;
	BrakingFrictionFactor_GUI = GetCharacterMovement()->BrakingFrictionFactor;
	MaxAcceleration_GUI = GetCharacterMovement()->MaxAcceleration;

	InitialMass = GetCharacterMovement()->Mass;
	InitialGroundFriction = GetCharacterMovement()->GroundFriction;
	InitialMaxAcceleration = GetCharacterMovement()->MaxAcceleration;
	InitialMaxSpeed = GetCharacterMovement()->MaxWalkSpeed;
	InitialRotationRate = GetCharacterMovement()->RotationRate;

	GameModeRef = Cast<AEMGGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

// Called every frame
void AEMGBasePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//***********************************************************************************
	if(!UEMGConsoleManager::bToggleWindowGUIDebug || !GetWorld()->IsServer())
	{
		return;
	}
	// Cache player nickname
	const FString playerNickname = FString(GetPlayerState()->GetPlayerName());
	
#pragma region Movement Debugging
	 {

	 	ImGui::Begin(TCHAR_TO_UTF8(*FString(playerNickname + " Shopping cart movement settings")), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	 	ImGui::SliderFloat("Rotation Rate Z", &RotationRateZ_GUI, 0, 360.f);
	 	ImGui::SliderFloat("Max Walk Speed", &MaxWalkSpeed_GUI, 0, 3000.f);
	 	ImGui::SliderFloat("Braking Deceleration Walking", &BrakingDecelerationWalking_GUI, 0.f, 5000.f);
	 	ImGui::SliderFloat("Ground Friction", &GroundFriction_GUI, 0, 10.f);
	 	ImGui::SliderFloat("Braking Friction Factor", &BrakingFrictionFactor_GUI, 0, 10.f);
	 	ImGui::SliderFloat("Max Acceleration", &MaxAcceleration_GUI, 0.f, 10000);
	 	ImGui::End();

	 	GetCharacterMovement()->RotationRate.Yaw = RotationRateZ_GUI;
	 	GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed_GUI;
	 	GetCharacterMovement()->BrakingDecelerationWalking = BrakingDecelerationWalking_GUI;
	 	GetCharacterMovement()->GroundFriction = GroundFriction_GUI;
	 	GetCharacterMovement()->BrakingFrictionFactor = BrakingFrictionFactor_GUI;
	 	GetCharacterMovement()->MaxAcceleration = MaxAcceleration_GUI;
#pragma endregion
		
		{
			

#pragma region Inventory Debugging
	 		ImGui::Begin(TCHAR_TO_UTF8(*FString(playerNickname + " Inventory Debugging")),nullptr,ImGuiWindowFlags_AlwaysAutoResize);
	 		if(ImGui::Button("Empty Inventory"))
	 		{
	 			InventoryComp->Server_EmptyInventory();
	 		}
	 		

	 		static int selectedDataTable = 0;
	 		const char* DataTables[] = {"LowValueItemsDT","MidValueItemsDT","HighValueItemsDT"};

	 		if(ImGui::BeginCombo("Item value",DataTables[selectedDataTable]))
	 		{
	 			for (int n = 0;n < IM_ARRAYSIZE(DataTables);n++)
	 			{
	 				bool isSelected = (selectedDataTable == n);
	 				if(ImGui::Selectable(DataTables[n], isSelected))
	 				{
	 					selectedDataTable = n;
	 				}
	 				
	 				if(isSelected){ImGui::SetItemDefaultFocus();}
	 			}

	 			ImGui::EndCombo();
	 		}
	 		
	 		if(ImGui::Button("Add Item to inventory"))
	 		{
	 			UDataTable* CurrentDataTable = nullptr;
	 			
		        switch (selectedDataTable)
		        {
					case 0: CurrentDataTable = LowValueItemsDT; break;
					case 1: CurrentDataTable = MidValueItemsDT; break;
					case 2: CurrentDataTable = HighValueItemsDT; break;
		        	
		        }
				if (!CurrentDataTable)
					return;
	 			
	 			TArray<FName> ItemsInDT = CurrentDataTable->GetRowNames();
	 			int32 NumberOfItems = ItemsInDT.Num();
	 			int32 RandomItemNumber = FMath::RandRange(0, NumberOfItems-1);
	 			
	 			if(!CurrentDataTable)
	 			{
	 				return;
	 			}
	 			
	 			FItemsInInventory* ItemToAdd = CurrentDataTable->FindRow<FItemsInInventory>(ItemsInDT[RandomItemNumber],TEXT("Looking for random item"));
	 			
	 			if(!ItemToAdd)
	 			{
	 				return;
	 			}
	 			
	 			FItemsInInventory newItem = *ItemToAdd;
	 			InventoryComp->Server_AddItemToInventory(newItem);
	 		}

	 		ImGui::End();
#pragma endregion
	 		
	 	}

#pragma region Powerup debugging
		
	 	{
	 		ImGui::Begin(TCHAR_TO_UTF8(*FString(playerNickname + " Powerup Debugging")),nullptr,ImGuiWindowFlags_AlwaysAutoResize);
	 		if(ImGui::Button("Give Speed Powerup"))
	 		{
	 			PowerupComponent->CurrentPowerup = EPowerUpType::SpeedBoost;
	 			bCanTriggerPowerUp = true;
	 		}
	 		
	 		if(ImGui::Button("Give Infinite Inventory Powerup"))
	 		{
	 			PowerupComponent->CurrentPowerup = EPowerUpType::InfiniteInventory;
	 			bCanTriggerPowerUp = true;
	 		}
	 		
	 		if(ImGui::Button("Give Shield Powerup"))
	 		{
	 			PowerupComponent->CurrentPowerup = EPowerUpType::Shield;
	 			bCanTriggerPowerUp = true;
	 		}
	 		
	 		if(ImGui::Button("Give Gooey Powerup"))
	 		{
	 			PowerupComponent->CurrentPowerup = EPowerUpType::Gooey;
	 			bCanTriggerPowerUp = true;
	 		}

	 		if(ImGui::Button("Give Slippery Powerup"))
	 		{
	 			PowerupComponent->CurrentPowerup = EPowerUpType::Slippery;
	 			bCanTriggerPowerUp = true;
	 		}
	 		
	 		ImGui::End();
	 		
#pragma endregion 

#pragma region Match Debugging
	 		
	 		{
	 			ImGui::Begin(TCHAR_TO_UTF8(*FString(playerNickname + " Match Debugging")),nullptr,ImGuiWindowFlags_MenuBar);
	 		
	 			if(!GameModeRef)
	 			{
	 				return;
	 			}
	 		
	 			if(ImGui::Button("Pause Timer"))
	 			{
	 				GetWorld()->GetTimerManager().PauseTimer(GameModeRef->Match_TimerHandle);
	 			}

	 			if(ImGui::Button("Resume Timer"))
	 			{
	 				GetWorld()->GetTimerManager().UnPauseTimer(GameModeRef->Match_TimerHandle);
	 			}
	 		
	 			ImGui::End();
	 			}
	 		}

#pragma endregion 
		
	 }
}

#pragma region Input 
void AEMGBasePlayer::MoveForward_Input(float AxisValue)
{
	if(!HasMatchStarted())
	{
		return;
	}
	
	if(bShouldUseVehicleControlScheme)
	{
		return;
	}
	
	if(CheckState(EActionState::CheckingOut) || CheckState(EActionState::Ramming))
	{
		return;
	}
	
	if(AxisValue == 0.f)
		return;
		
	const float yaw = CameraComp != nullptr ? CameraComp->GetComponentRotation().Yaw : 1.F;
	const FRotator forwardRotation = FRotator(0, yaw,0);
	const FVector dirToGo = FRotationMatrix(forwardRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(dirToGo * AxisValue);
	Server_SetCurrentState(EActionState::Movement);
}

void AEMGBasePlayer::MoveRight_Input(float AxisValue)
{
	if(!HasMatchStarted())
	{
		return;
	}
	
	if(bShouldUseVehicleControlScheme)
	{
		return;
	}
	
	if(CheckState(EActionState::CheckingOut) || CheckState(EActionState::Ramming))
	{
		return;
	}
	
	if(AxisValue == 0.f)
		return;

	const float yaw = CameraComp != nullptr ? CameraComp->GetComponentRotation().Yaw : 1.F;
	const FRotator forwardRotation = FRotator(0, yaw,0);
	const FVector dirToGo = FRotationMatrix(forwardRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(dirToGo * AxisValue);
	Server_SetCurrentState(EActionState::Movement);
}

void AEMGBasePlayer::Ram_Input()
{
	if(!HasMatchStarted())
	{
		return;
	}
	
	if(CheckState(EActionState::CheckingOut))
		return;
	
	if(!RamAbilityComp)
		return;

	if(!RamAbilityComp->GetCanUseAbility())
	{
		return;
	}

	Server_ResetState(EActionState::Movement); // set so character is not on the moving state
	RamAbilityComp->UseAbility();
}

void AEMGBasePlayer::Interact_Input()
{
	
}

void AEMGBasePlayer::Accelerate_Input(float AxisValue)
{
	if(!bShouldUseVehicleControlScheme)
    	{
    		return;
    	}
	if(CheckState(EActionState::CheckingOut) || CheckState(EActionState::Ramming))
	{
		return;
	}

	if(AxisValue == 0.f)
		return;

	const FRotator forwardRotation = FRotator(0, GetActorRotation().Yaw,0);
	const FVector dirToGo = FRotationMatrix(forwardRotation).GetUnitAxis(EAxis::X);
	
	AddMovementInput(dirToGo * AxisValue);
	Server_SetCurrentState(EActionState::Movement);
}

void AEMGBasePlayer::Brake_Input(float AxisValue)
{
	if(!bShouldUseVehicleControlScheme)
	{
		return;
	}
	if(CheckState(EActionState::CheckingOut) || CheckState(EActionState::Ramming))
	{
		return;
	}

	if(AxisValue == 0.f)
		return;

	const FRotator forwardRotation = FRotator(0, GetActorRotation().Yaw,0);
	const FVector dirToGo = FRotationMatrix(forwardRotation).GetUnitAxis(EAxis::X);
	
	AddMovementInput(dirToGo * AxisValue);
	Server_SetCurrentState(EActionState::Movement);
}

void AEMGBasePlayer::Turn_Input(float AxisValue)
{
	if(!bShouldUseVehicleControlScheme)
	{
		return;
	}
	
	float rotSpeed = TurnRotationSpeed * AxisValue;
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green, FString::Printf(TEXT("Axis: %f"), rotSpeed));
	}
	AddActorWorldRotation(FRotator(0, TurnRotationSpeed, 0.f));
}


#pragma endregion



void AEMGBasePlayer::AddShoppingItemToInventory_Implementation(FItemsInInventory itemToAdd)
{
	IEMGPlayerInterface::AddShoppingItemToInventory_Implementation(itemToAdd);
}


/********************* Inherited interfaces *********************/

bool AEMGBasePlayer::HasItems_Implementation()
{
	if(!InventoryComp)
		return false;
	
	return InventoryComp->PlayerInventory.Num() > 0;
	
}

void AEMGBasePlayer::StartedCheckingOut_Implementation()
{
	IEMGPlayerInterface::StartedCheckingOut_Implementation();
	
}

void AEMGBasePlayer::CompletedCheckingOut_Implementation()
{
	IEMGPlayerInterface::CompletedCheckingOut_Implementation();
	
	check(GetPlayerState());
	check(InventoryComp);		

	float TotalItemsValue = 0.f;

	// iterate through every item inside current player inventory to add it to the TotalItemsValue
	for(const auto& Item : InventoryComp->PlayerInventory)
	{
		TotalItemsValue += Item.Value;
	}

	// Get current score and add it with total items value found
	const float CurrentScore = GetPlayerState()->GetScore();
	GetPlayerState()->SetScore(CurrentScore + TotalItemsValue);
	//Server_ResetState(EActionState::CheckingOut);
	InventoryComp->Server_EmptyInventory();
}

void AEMGBasePlayer::InterruptedCheckingOut_Implementation()
{
	IEMGPlayerInterface::InterruptedCheckingOut_Implementation();
}

void AEMGBasePlayer::OnGettingRammed_Implementation()
{
	IEMGPlayerInterface::OnGettingRammed_Implementation();

}


/********************* Network events *********************/

void AEMGBasePlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentActionState);	
}

void AEMGBasePlayer::Server_SetCurrentState_Implementation(EActionState state)
{
	Multicast_SetCurrentState(state);
}

void AEMGBasePlayer::Multicast_SetCurrentState_Implementation(EActionState state)
{
	// turn on that particular bit
	CurrentActionState |= static_cast<int>(state);	
}


void AEMGBasePlayer::Server_ResetState_Implementation(EActionState state)
{
	Multicast_ResetState(state);
}

void AEMGBasePlayer::Multicast_ResetState_Implementation(EActionState state)
{
	// turn off that particular bit
	CurrentActionState &= (~static_cast<int>(state));	
}


bool AEMGBasePlayer::HasMatchStarted() const
{
	if(AEMGGameState* gameState = Cast<AEMGGameState>( UGameplayStatics::GetGameState(GetWorld())  ))
	{
		return gameState->bMatchRunning;
	}
	return true;
}