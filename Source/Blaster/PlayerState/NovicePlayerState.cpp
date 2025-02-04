// Fill out your copyright notice in the Description page of Project Settings.


#include "NovicePlayerState.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Net/UnrealNetwork.h"



void ANovicePlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	//for setting the score on server should be incharge of the score the sccore is a replicated variable so it will be replicated down to all clients through OnRep_score repNotify
	NoviceCharacter = NoviceCharacter == nullptr ? Cast<ANoviceCharacter>(GetPawn()) : NoviceCharacter;
	if (NoviceCharacter)
	{
		Controller = Controller == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());//Once the score is replicated we want to update the HUD,calling on the server
		}
	}
}

void ANovicePlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	//for setting the score on server should be incharge of the score the sccore is a replicated variable so it will be replicated down to all clients through OnRep_score repNotify
	NoviceCharacter = NoviceCharacter == nullptr ? Cast<ANoviceCharacter>(GetPawn()) : NoviceCharacter;
	if (NoviceCharacter)
	{
		Controller = Controller == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);//Once the score is replicated we want to update the HUD,calling on the server
		}
	}
}

void ANovicePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//Register the variable that you want to replicate here in this function

	DOREPLIFETIME(ANovicePlayerState, Defeats);
	DOREPLIFETIME(ANovicePlayerState,Team);
}

void ANovicePlayerState::OnRep_Defeats()
{
	NoviceCharacter = NoviceCharacter == nullptr ? Cast<ANoviceCharacter>(GetPawn()) : NoviceCharacter;
	if (NoviceCharacter)
	{
		Controller = Controller == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);//Once the score is replicated we want to update the HUD,calling on the server
		}
	}
}

void ANovicePlayerState::OnRep_Score()
{

	NoviceCharacter=NoviceCharacter==nullptr ? Cast<ANoviceCharacter>(GetPawn()) : NoviceCharacter;
	if (NoviceCharacter)
	{
		Controller=Controller==nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());//Once the score is replicated we want to update the HUD,calling on the client
		}
	}
}

void ANovicePlayerState::OnRep_Team()
{
	ANoviceCharacter*  Character = Cast<ANoviceCharacter>(GetPawn());
	if ( Character)
	{
		 Character->SetTeamColor(Team);
	}
}

void ANovicePlayerState::SetTeam(ETeams TeamToSet)
{
	 Team = TeamToSet; 
	 ANoviceCharacter*  Character = Cast<ANoviceCharacter>(GetPawn());
	 if ( Character)
	 {
		  Character->SetTeamColor(TeamToSet);
	 }
}


 