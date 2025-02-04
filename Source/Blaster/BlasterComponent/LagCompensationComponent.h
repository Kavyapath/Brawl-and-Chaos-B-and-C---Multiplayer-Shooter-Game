// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};


USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	TMap<FName, FBoxInformation> HitBoxInfo;
	UPROPERTY()
	ANoviceCharacter* Character;//only using it for the Shotgun SSR
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;

};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ANoviceCharacter*, uint32> HeadShots;
	UPROPERTY()
	TMap<ANoviceCharacter*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	ULagCompensationComponent();
	friend class ANoviceCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SaveFramePackageInTick();
	void ShowFramePackage(const FFramePackage& Package,const FColor& Color);
	/*
	Server Side Rewind for HitScanWeapon
	*/
	FServerSideRewindResult ServerSideRewind(class ANoviceCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation,
		float HitTime);


	UFUNCTION(Server,Reliable)
	void ServerScoreRequest(
		ANoviceCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime,
		class AWeapon* DamageCauser
	);

	/*
	Server Side Rewind for shotgunWeapon
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ANoviceCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<ANoviceCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime,
		AWeapon* DamageCauser);

	/*
	Server Side Rewind for ProjectileWeapon
	*/
	FServerSideRewindResult ProjectileServerSideRewind(ANoviceCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);


	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(ANoviceCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime);

protected:
	
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(ANoviceCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ANoviceCharacter* HitCharacter,const FFramePackage& Package);
	void ResetHitBoxes(ANoviceCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ANoviceCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	FFramePackage GetFrameToCheck(ANoviceCharacter* HitCharacter, float HitTime);
	/* 
	 HitScanWeapon
	*/
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ANoviceCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	/*
	 ProjectileWeapon
	*/

	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ANoviceCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime);
	/*
	Shotgun
	*/
	
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations);
private:


	UPROPERTY()
	ANoviceCharacter* NoviceCharacter;

	UPROPERTY()
	class ANovicePlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;//limit of linklist to stroe the time

public:	
	
	
		
};
