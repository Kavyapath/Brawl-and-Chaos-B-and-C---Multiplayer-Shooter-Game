// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UBuffComponent();
	friend class ANoviceCharacter;
	void Heal(float HealAmount,float HealingTime);
	void SetInitialSpeed(float BaseSpeed,float CrouchSpeed);
	void SetInitialJumpSpeed(float BaseJumpVelocity);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	void BuffJump(float BuffJumpVelocity, float BuffTime);
protected:

	virtual void BeginPlay() override;
	void HealRampUp(float DeltaTime);
	void ShieldRampUp(float DeltaTime);
public:	

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:

	UPROPERTY()
	class ANoviceCharacter* NoviceCharacter;

	/*
	Health Buff
	*/
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
	/*
	Speed Buff
	*/

	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();

	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/*
	Jump Buff
	*/

	FTimerHandle JumpBuffTimer;

	void ResetJump();
	float InitialJumpVelocity;
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

	/*
	Shield Replenishing Time
	*/
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount=0.f;

};
