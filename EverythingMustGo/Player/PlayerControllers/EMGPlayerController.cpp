// Copyright (c) 2023 @ Runaway Concepts - Pedro E. Perez, Rafael Zagolin


#include "EMGPlayerController.h"

#include "EverythingMustGo/Player/EMGBasePlayer.h"
#include "EverythingMustGo/GameInstances/EMGGameInstance.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

#include <imgui.h>

#include "EMGGameMode.h"
#include "EverythingMustGo/EMGConsoleManager.h"
#include "EverythingMustGo/Player/Components/EMGPlayerInventory.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"


#define LOG(time, text) if(GEngine) \
GEngine->AddOnScreenDebugMessage(-1, time, FColor::Green, FString(text));

// Sets default values
AEMGPlayerController::AEMGPlayerController()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}



// Called when the game starts or when spawned
void AEMGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Assign server player controller reference for the game mode
	if(GetNetMode() == NM_ListenServer)
	{
		AEMGGameMode* GameModeRef = Cast<AEMGGameMode>( UGameplayStatics::GetGameMode(GetWorld()->GetAuthGameMode()));
		if(GameModeRef)
		{
			GameModeRef->ServerPlayerController = this;
		}
	}
}

// Called every frame
void AEMGPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#pragma region Debug player Stats
	// Debug player stats
	/*if(!UEMGConsoleManager::bToggleWindowGUIDebug)
	{
		return;
	}	
	{		
		const FString WindowName = PlayerState->GetPlayerName() + " Info"; 
		ImGui::Begin(TCHAR_TO_UTF8(*WindowName), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		{
			// Match state
			if(MatchTimeLeft > 0)
			{
				// Convert the int to FString
				FString StringConv = FString::Printf(TEXT("%d"), MatchTimeLeft);
			
				// Convert FString to const char*
				const char* CharPtr = TCHAR_TO_UTF8(*StringConv);
			
				ImGui::LabelText(CharPtr, "Match time: "); // players that currently connected to match (entered the level)
			}
			else
			{			
				// Convert FString to const char*
				const char* CharPtr = TCHAR_TO_UTF8(*WinnerName);
			
				ImGui::LabelText(CharPtr, "Match over, winner is "); // players that currently connected to match (entered the level)
			}
			
			{
				// Convert the int to FString
				FString IntAsString = FString::Printf(TEXT("%f"), GetPlayerState<APlayerState>()->GetScore());
			
				// Convert FString to const char*
				const char* CharPtr = TCHAR_TO_UTF8(*IntAsString);
			
				ImGui::LabelText(CharPtr, "Current Score: "); // players that currently connected to match (entered the level)
			}

			// Show current player inventory
			{
				if(AEMGBasePlayer* BasePlayer = Cast<AEMGBasePlayer>(GetPawn()))
				{
					if(UEMGPlayerInventory* Inventory = BasePlayer->InventoryComp)
					{
						for (auto _Inventory : Inventory->PlayerInventory)
						{							
							// Convert FString to const char*
							const char* CharItem = TCHAR_TO_UTF8(*_Inventory.ItemType);
			
							ImGui::LabelText(CharItem, "Item: "); // players that currently connected to match (entered the level)							
							
							
							// Convert the float to FString
							FString Value = FString::Printf(TEXT("%f"), _Inventory.Value);
							
							// Convert FString to const char*
							const char* CharValue = TCHAR_TO_UTF8(*Value);
			
							ImGui::LabelText(CharValue, "Value: "); // players that currently connected to match (entered the level)
						}
					}
				}
				
			}
		}
			
		ImGui::End();*/
	//}
	
	/*if (GetWorld()->GetNetMode() == ENetMode::NM_ListenServer)
	{
		{
			ImGui::Begin(TCHAR_TO_UTF8(*PlayerState->GetName()), nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::Button("Restart Session"))
			{
				// hard coding the Game_Level name here which is quite dangerous but we'll change it later
				UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), "servertravel Game_Level");
			}
			if (ImGui::Button("Quit"))
			{
				if( UEMGGameInstance* GameInstance = Cast<UEMGGameInstance>( UGameplayStatics::GetGameInstance(GetWorld() ) ) )
				{
					GameInstance->DestroySession();
				}
			}
			
			ImGui::End();
		}
	}*/
#pragma endregion
	
}

void AEMGPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	
}

void AEMGPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AEMGPlayerController, MatchTimeLeft);	
	DOREPLIFETIME(AEMGPlayerController, WinnerName);	
	DOREPLIFETIME(AEMGPlayerController, PlayerInfo);	
}

void AEMGPlayerController::OnRep_PlayerInfo()
{
	// Assign info on possessed player
	BPIE_AssignPlayerInfo(PlayerInfo);
	//this->PlayerInfo = _PlayerInfo;
}

void AEMGPlayerController::AssignPlayerInfo(FPlayerInfo _PlayerInfo)
{
	if(HasAuthority())
		this->PlayerInfo = _PlayerInfo;
}

