// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();

	void ResetFlag();
protected:
	virtual void BeginPlay() override;
	virtual void Dropped() override;
	virtual void OnDropped() override;
	virtual void OnEquipped() override;
private: 
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;
	
public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
};
