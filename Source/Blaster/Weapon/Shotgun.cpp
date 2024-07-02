// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


void AShotgun::Fire(const FVector& HitTarget)
{

	AWeapon::Fire(HitTarget);
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
	
		FHitResult FireHit;
		TMap<ANoviceCharacter*, uint32> HitMap;

		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			
			WeaponTraceHit(Start,HitTarget,FireHit);

			ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(FireHit.GetActor());
			if (NoviceCharacter && HasAuthority() && GetInstigatorController())
			{
				if (HitMap.Contains(NoviceCharacter))
				{
					HitMap[NoviceCharacter]++;
				}
				else
				{
					HitMap.Emplace(NoviceCharacter,1);//as the no of hits
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
					FMath::RandRange(-.5f,.5f));
			}
		}
		for (auto it : HitMap)
		{
			if (it.Key && HasAuthority() && GetInstigatorController())
			{


				UGameplayStatics::ApplyDamage(
					it.Key,
					Damage * it.Value,
					GetInstigatorController(),
					this,
					UDamageType::StaticClass()
				);
			}
		}
		

		
	}

}
