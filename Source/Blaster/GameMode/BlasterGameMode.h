// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"


namespace MatchState
{
	extern BLASTER_API const FName Cooldown; //Match duration have been reached.Display winner and begin cooldown timer
}
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	ABlasterGameMode();
	virtual void  PlayerEliminated(class ANoviceCharacter* ElimmedCharacter,class ANovicePlayerController* VictimController,class ANovicePlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter,AController* ElimmedController);
	virtual float CalculateDamage(AController* Attacker, AController* Victim,float BaseDamage);
	void PlayerLeftGame(class ANovicePlayerState* PlayerLeaving);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;

	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
