// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/NovicePlayerState.h"
#include <Blaster/PlayerController/NovicePlayerController.h>

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayer);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(ANovicePlayerState* ScoringPlayer)
{
	if (TopScoringPlayer.Num() == 0)
	{
		TopScoringPlayer.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore()==TopScore)
	{
		TopScoringPlayer.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayer.Empty();
		TopScoringPlayer.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	
}

void ABlasterGameState::RedTeamScoreBoard()
{
	++RedTeamScore;
	ANovicePlayerController* NovicePlayerController = Cast<ANovicePlayerController>(GetWorld()->GetFirstPlayerController());
	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDRedTeamScore( RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScoreBoard()
{
	++BlueTeamScore;
	ANovicePlayerController* NovicePlayerController = Cast<ANovicePlayerController>(GetWorld()->GetFirstPlayerController());
	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDBlueTeamScore( BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ANovicePlayerController* NovicePlayerController = Cast<ANovicePlayerController>(GetWorld()->GetFirstPlayerController());
	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDRedTeamScore( RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ANovicePlayerController* NovicePlayerController = Cast<ANovicePlayerController>(GetWorld()->GetFirstPlayerController());
	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDBlueTeamScore( BlueTeamScore);
	}
}
