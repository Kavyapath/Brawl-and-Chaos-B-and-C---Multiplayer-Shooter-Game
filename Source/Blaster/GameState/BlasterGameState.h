// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;
	void UpdateTopScore(class ANovicePlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
	TArray<ANovicePlayerState*> TopScoringPlayer;

	/*
	Teams
	*/
	TArray<ANovicePlayerState*> RedTeam;
	TArray<ANovicePlayerState*> BlueTeam;

	void RedTeamScoreBoard();
	void BlueTeamScoreBoard();

	UPROPERTY(ReplicatedUsing= OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;
	UFUNCTION()
	void OnRep_RedTeamScore();
	UFUNCTION()
	void OnRep_BlueTeamScore();


private:
	float TopScore=0.f;
	
};
