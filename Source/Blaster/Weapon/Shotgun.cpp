// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"



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
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//maps hit character to number of times hit
		TMap<ANoviceCharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(FireHit.GetActor());
			if (NoviceCharacter )
			{
				if (HitMap.Contains(NoviceCharacter))
				{
					HitMap[NoviceCharacter]++;
				}
				else
				{
					HitMap.Emplace(NoviceCharacter, 1);//as the no of hits
				}

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
			
			for (auto it : HitMap)
			{
				if (it.Key && HasAuthority() && GetInstigatorController())
				{


					UGameplayStatics::ApplyDamage(
						it.Key,//character that was hit
						Damage * it.Value,//multiplying damage by no of times hit
						GetInstigatorController(),
						this,
						UDamageType::StaticClass()
					);
				}
			}

		}
		
	}
}

