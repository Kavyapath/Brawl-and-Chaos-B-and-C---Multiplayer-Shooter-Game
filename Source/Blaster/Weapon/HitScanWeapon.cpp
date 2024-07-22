// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation() ;
		FVector End = Start + (HitTarget-Start) * 1.25f;
		FHitResult FireHit;
	
			WeaponTraceHit(Start, End, FireHit);

			ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(FireHit.GetActor());
			if (NoviceCharacter && HasAuthority() && GetInstigatorController())
			{

				UGameplayStatics::ApplyDamage(
					NoviceCharacter,
					Damage,
					GetInstigatorController(),
					this,
					UDamageType::StaticClass()
				);

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
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
			}
		

			 
		

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound,GetActorLocation());

		}
	}

}



void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& TraceHitTarget,FHitResult& OutHit)
{

	
	UWorld* World = GetWorld();
	if (World)
	{

		FVector End =  TraceStart + (TraceHitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);





		if (!OutHit.bBlockingHit)
		{
			OutHit.ImpactPoint = End;
		}
		DrawDebugSphere(GetWorld(), OutHit.ImpactPoint,16.f,12,FColor::Orange,true);

		if (BeamParticle)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticle,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);

			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), OutHit.ImpactPoint);

			}
		}
	}

}
 

