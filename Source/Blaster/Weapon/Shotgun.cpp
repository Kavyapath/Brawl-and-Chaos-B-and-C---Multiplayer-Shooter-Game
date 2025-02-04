// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/PlayerController/NovicePlayerController.h"



void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();

	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; i++)
	{

		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);

		const FVector EndLoc = SphereCenter + RandVec;

		 FVector ToEndLoc = EndLoc - TraceStart;

		 ToEndLoc = TraceStart + ToEndLoc * Trace_Lenght / ToEndLoc.Size();
		 HitTargets.Add(ToEndLoc);
	}
}

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//maps hit character to number of times hit
		TMap<ANoviceCharacter*, uint32> HitMap;
		TMap<ANoviceCharacter*, uint32> HeadShotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(FireHit.GetActor());
			if (NoviceCharacter )
			{

				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head") || FireHit.BoneName.ToString()== FString("PonyTail") || FireHit.BoneName.ToString()==FString("bangs");
				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(NoviceCharacter)) HeadShotHitMap[NoviceCharacter]++;
					else HeadShotHitMap.Emplace(NoviceCharacter, 1);//as the no of hits
				}
				else
				{
					if (HitMap.Contains(NoviceCharacter)) HitMap[NoviceCharacter]++;
					else HitMap.Emplace(NoviceCharacter, 1);//as the no of hits
				}

 
				if (ImpactParticle)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticle,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this,
						HitSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::RandRange(-.5f, .5f));
				}

				TArray<ANoviceCharacter*> HitCharacters;
				//Maps Character hit to total Damage
				TMap<ANoviceCharacter*, float> DamageMap;

				//Calculate bodyshot damage by multiplying times hit* Damage - Store in damage map
				for (auto it : HitMap)
				{

					if (it.Key  )
					{
						
						DamageMap.Emplace(it.Key, it.Value * Damage);
						HitCharacters.AddUnique(it.Key);

					}
				}
				//Calculate headshot damage by multiplying times hit* HeadShotDamage - Store in damage map
				for (auto it : HeadShotHitMap)
				{

					if (it.Key )
					{

						if (DamageMap.Contains(it.Key))  DamageMap[it.Key] += it.Value*HeadShotDamage;
						else DamageMap.Emplace(it.Key, it.Value * HeadShotDamage);
						HitCharacters.AddUnique(it.Key);

					}
				}

				//Loop through damage map to get total damage for each character
				for (auto DamagePair : DamageMap)
				{
					if (DamagePair.Key && GetInstigatorController())
					{
						bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
						if (HasAuthority() && bCauseAuthDamage )
						{
							UGameplayStatics::ApplyDamage(
								DamagePair.Key,//character that was hit
								DamagePair.Value,//Damage Calculated in the two for loop
								GetInstigatorController(),
								this,
								UDamageType::StaticClass()
							);
						}
					}
				}
				

				if (!HasAuthority() && bUseServerSideRewind)
				{
					NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;
					NoviceOwnerController = NoviceOwnerController == nullptr ? Cast<ANovicePlayerController>(GetInstigatorController()) : NoviceOwnerController;
					if (NoviceOwnerCharacter && NoviceOwnerController && NoviceOwnerCharacter->GetLagCompensationComponent() && NoviceOwnerCharacter->IsLocallyControlled())
					{
						NoviceOwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(HitCharacters, Start, HitTargets, NoviceOwnerController->GetServerTime() - NoviceOwnerController->SingleTripTime, this);
					}

				}

			}
			
		}
		
	}
}

