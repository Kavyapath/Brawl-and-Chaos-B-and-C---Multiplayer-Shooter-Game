// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()


protected:
	virtual void Fire(const FVector& HitTarget) override;
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& TraceStart, const FVector& TraceHitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	 UParticleSystem* ImpactParticle;

	 UPROPERTY(EditAnywhere)
	 UParticleSystem* BeamParticle;

	 UPROPERTY(EditAnywhere)
	 UParticleSystem* MuzzleFlash;

	 UPROPERTY(EditAnywhere)
	 USoundCue* HitSound;

	 UPROPERTY(EditAnywhere)
	 USoundCue* FireSound;

	 /*
	 Trace end with scatter
	 
	 */

	 UPROPERTY(EditAnywhere,Category="Weapon Scatter")
	 float DistanceToSphere = 800.f;


	 UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	 float SphereRadius = 75.f;


	 UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	 bool bUseScatter = false;

	
};