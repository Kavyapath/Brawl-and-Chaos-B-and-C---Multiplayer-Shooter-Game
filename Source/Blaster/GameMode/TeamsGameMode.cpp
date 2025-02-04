// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/NovicePlayerState.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Kismet/GameplayStatics.h"


ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		for (auto PState : BGameState->PlayerArray)
		{
			ANovicePlayerState* NPState = Cast<ANovicePlayerState>(PState.Get());
			if (NPState && NPState->GetTeam() == ETeams::ET_NoTeam)
			{
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(NPState);
					NPState->SetTeam(ETeams::ET_RedTeam);

				}
				else
				{
					BGameState->BlueTeam.AddUnique(NPState);
					NPState->SetTeam(ETeams::ET_BlueTeam);

				}
			}
		}
		
	}
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);


	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{

	
		ANovicePlayerState* NPState = NewPlayer->GetPlayerState<ANovicePlayerState>();
		if (NPState && NPState->GetTeam() == ETeams::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(NPState);
				NPState->SetTeam(ETeams::ET_RedTeam);

			}
			else
			{
				BGameState->BlueTeam.AddUnique(NPState);
				NPState->SetTeam(ETeams::ET_BlueTeam);

			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ANovicePlayerState* NPState = Exiting->GetPlayerState<ANovicePlayerState>();

	if (BGameState && NPState)
	{

		if (BGameState->RedTeam.Contains(NPState))
		{
			BGameState->RedTeam.Remove(NPState);
		}

		if (BGameState->BlueTeam.Contains(NPState))
		{
			BGameState->BlueTeam.Remove(NPState);
		}
  	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ANovicePlayerState* AttackerPlayerState =  Attacker->GetPlayerState<ANovicePlayerState>();
	ANovicePlayerState* VictimPlayerState = Victim->GetPlayerState<ANovicePlayerState>();

	if (AttackerPlayerState == nullptr || VictimPlayerState == nullptr)
	{
		return BaseDamage;
	}
	if (AttackerPlayerState->GetTeam() == VictimPlayerState->GetTeam())
	{
		return 0.f;
	}
	return BaseDamage;
}

void ATeamsGameMode::PlayerEliminated(ANoviceCharacter* ElimmedCharacter, ANovicePlayerController* VictimController, ANovicePlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

	ANovicePlayerState* AttackerPlayerState = AttackerController ? Cast<ANovicePlayerState>(AttackerController->PlayerState) : nullptr;

	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeams::ET_BlueTeam)
		{
			BGameState->BlueTeamScoreBoard();
		}
		if (AttackerPlayerState->GetTeam() == ETeams::ET_RedTeam)
		{
			BGameState->RedTeamScoreBoard();
		}
		
	}

}
