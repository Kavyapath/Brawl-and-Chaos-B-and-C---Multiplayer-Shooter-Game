// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"


void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(MultiplayerSessionsSubsystem);
		if (NumberOfPlayers == MultiplayerSessionsSubsystem->DesiredNumPublicConnection) {

			UWorld* World = GetWorld();
			if (World) {
				bUseSeamlessTravel = true;

				FString MatchType = MultiplayerSessionsSubsystem->DesiredMatchType;

				if (MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMapTeams?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Maps/BlasterMapCaptureTheFlag?listen"));
				}


			}
		}
	}


	


}

void ALobbyGameMode::StartGameFromServer()
{

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(MultiplayerSessionsSubsystem);
		UWorld* World = GetWorld();
		if (World) {
			bUseSeamlessTravel = true;

			FString MatchType = MultiplayerSessionsSubsystem->DesiredMatchType;

			if (MatchType == "FreeForAll")
			{
				World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
			}
			else if (MatchType == "Teams")
			{
				World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
			}
			else if (MatchType == "CaptureTheFlag")
			{
				World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
			}
			

		}
	}


}
