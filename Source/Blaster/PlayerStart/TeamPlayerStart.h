// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/Enums/Trams.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere)
	ETeams Team;
	
};
