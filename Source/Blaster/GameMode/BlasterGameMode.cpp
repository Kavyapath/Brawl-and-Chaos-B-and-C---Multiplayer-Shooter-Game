// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Kismet/GameplayStatics.h" 
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/NovicePlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{

	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
} 

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{

			StartMatch();//already existed function on the GameMode for starting the match
		}
	}
	else if (MatchState==MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime =CooldownTime+ WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}

}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)//iterarting through all the player controller in our game
	{
		ANovicePlayerController* NovicePlayerController = Cast<ANovicePlayerController>(*it);
		if (NovicePlayerController)
		{
			NovicePlayerController->OnMatchStateSet(MatchState,bTeamsMatch);
		}
	}
}


void ABlasterGameMode::PlayerEliminated(ANoviceCharacter* ElimmedCharacter, ANovicePlayerController* VictimController, ANovicePlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ANovicePlayerState* AttackerPlayerState = AttackerController ? Cast<ANovicePlayerState>(AttackerController->PlayerState) : nullptr;
	ANovicePlayerState* VictimPlayerState = VictimController ? Cast<ANovicePlayerState>(VictimController->PlayerState) : nullptr;
	

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ANovicePlayerState*> PlayerCurrentlyInLead;

		for (auto LeadPlayer : BlasterGameState->TopScoringPlayer)
		{
			PlayerCurrentlyInLead.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if (BlasterGameState->TopScoringPlayer.Contains(AttackerPlayerState))
		{
			ANoviceCharacter* Leader = Cast<ANoviceCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayerCurrentlyInLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayer.Contains(PlayerCurrentlyInLead[i]))//After update the top Scoring Array is the player is not present in this array so it means they lost the lead
			{
				ANoviceCharacter* Loser = Cast<ANoviceCharacter>(PlayerCurrentlyInLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				 }
			}
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1.f);
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; it++)
	{
		ANovicePlayerController* NovicePlayer = Cast<ANovicePlayerController>(*it);
		if (NovicePlayer && AttackerPlayerState && VictimPlayerState)
		{
			NovicePlayer->BroadcastElim(AttackerPlayerState,VictimPlayerState);
		}

	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();//one of the thing that this function does is detaches the player from the controller
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(),PlayerStarts );//to accessing the components in the world
		int32 Selection = FMath::RandRange(0,PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(ElimmedController,PlayerStarts[Selection]);//now we are restarting the player at random location from our array of player starts
	}
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{

	return BaseDamage;
}

void ABlasterGameMode::PlayerLeftGame(ANovicePlayerState* PlayerLeaving)
{
	//TODO Call elim and passing true for bLeft the game
	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayer.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayer.Remove(PlayerLeaving);
	}
	ANoviceCharacter* NoviceCharacter=Cast<ANoviceCharacter>(PlayerLeaving->GetPawn());
	if (NoviceCharacter)
	{
		NoviceCharacter->Elim(true);//as the character is leaving the game
	}
}

