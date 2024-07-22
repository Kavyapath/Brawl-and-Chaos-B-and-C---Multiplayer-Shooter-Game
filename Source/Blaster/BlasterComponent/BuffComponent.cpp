// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal+= HealAmount ;


}

void UBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpSpeed(float BaseJumpVelocity)
{
	InitialJumpVelocity = BaseJumpVelocity;
}

void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (NoviceCharacter == nullptr) return;

	NoviceCharacter->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&ThisClass::ResetSpeed,
		BuffTime);//this will be called after 30 sec has passed
	if (NoviceCharacter->GetCharacterMovement())
	{
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed= BuffBaseSpeed;
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);

}

void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	bReplenishingShield = true;
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	ShieldReplenishAmount += ShieldAmount;
}

void UBuffComponent::ResetSpeed()
{
	if (NoviceCharacter==nullptr || NoviceCharacter->GetCharacterMovement() == nullptr) return;
	
	NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	NoviceCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	

	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	if (NoviceCharacter == nullptr) return;
	NoviceCharacter->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&ThisClass::ResetJump,
		BuffTime);

	if (NoviceCharacter->GetCharacterMovement())
	{
		NoviceCharacter->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	MulticastJumpBuff(BuffJumpVelocity);

}

void UBuffComponent::ResetJump()
{
	if (NoviceCharacter->GetCharacterMovement())
	{
		NoviceCharacter->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	MulticastJumpBuff(InitialJumpVelocity);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	
	if (NoviceCharacter && NoviceCharacter->GetCharacterMovement())
	{
		NoviceCharacter->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (NoviceCharacter && NoviceCharacter->GetCharacterMovement())
	{
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;

	}

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

	
	
}

void UBuffComponent::HealRampUp(float DeltaTime)
{

	if (!bHealing || NoviceCharacter == nullptr || NoviceCharacter->IsElimmed() ) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	NoviceCharacter->SetHealth(FMath::Clamp(NoviceCharacter->GetHealth() + HealThisFrame,0.f, NoviceCharacter->GetMaxHealth()));
	NoviceCharacter->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;


	if (AmountToHeal <= 0.f || NoviceCharacter->GetHealth() >= NoviceCharacter->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}


}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bReplenishingShield || NoviceCharacter==nullptr || NoviceCharacter->IsElimmed()) return;

	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	
	NoviceCharacter->SetShield(FMath::Clamp(NoviceCharacter->GetShield() + ReplenishThisFrame,0.f,NoviceCharacter->GetMaxHealth()));
	
	NoviceCharacter->UpdateHUDShield();
	ShieldReplenishAmount -= ReplenishThisFrame;

	if (ShieldReplenishAmount <= 0.f || NoviceCharacter->GetShield() >= NoviceCharacter->GetMaxShield())
	{
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	HealRampUp(DeltaTime);
	ShieldRampUp(DeltaTime);
	
}



