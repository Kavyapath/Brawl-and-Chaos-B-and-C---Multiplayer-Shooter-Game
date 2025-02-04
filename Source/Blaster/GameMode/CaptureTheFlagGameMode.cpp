// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ANoviceCharacter* ElimmedCharacter, ANovicePlayerController* VictimController, ANovicePlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (GameState)
	{
		if (Zone->Team == ETeams::ET_BlueTeam)
		{
			BlasterGameState->BlueTeamScoreBoard();
		}
		if (Zone->Team == ETeams::ET_RedTeam)
		{
			BlasterGameState->RedTeamScoreBoard();
		}

	}
}
