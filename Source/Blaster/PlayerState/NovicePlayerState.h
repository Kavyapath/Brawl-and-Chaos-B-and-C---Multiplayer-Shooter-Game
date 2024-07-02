// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NovicePlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ANovicePlayerState : public APlayerState
{
	GENERATED_BODY()


	
public:

	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatsAmount);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	virtual void OnRep_Score() override;
private:
	UPROPERTY()
	class ANoviceCharacter* NoviceCharacter;//they are unintialize ptr they are initialize in begin play so giving the UPROPERTY() will initialize from null

	UPROPERTY()
	class ANovicePlayerController* Controller;//they are unintialize ptr they are initialize in begin play so giving the UPROPERTY() will initialize from null

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;
	
};
