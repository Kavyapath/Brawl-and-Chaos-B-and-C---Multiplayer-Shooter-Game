// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Blaster/Weapon/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	
	const USkeletalMeshSocket* MuzzleFlashSocket=GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		FTransform SocketTransform=MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//from muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();// to find the rotation of target vector
		
		APawn* InstigatorPawn = GetInstigator();
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;

			AProjectile* SpawnedProjectile=nullptr;
			if (bUseServerSideRewind)
			{
				if (InstigatorPawn->HasAuthority())//Server
				{
					if (InstigatorPawn->IsLocallyControlled())//Server, host - use replicated Projectile
					{
						SpawnedProjectile=World->SpawnActor<AProjectile>(ProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);

						SpawnedProjectile->bUseServerSideRewind = false;
						SpawnedProjectile->Damage = Damage;

					}
					else//Server, not locally controlled - Spawn non replicated projectile, Projectile SSR
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);

						SpawnedProjectile->bUseServerSideRewind = true;
						 
					}

				}
				else//client, using SSR
				{
					if (InstigatorPawn->IsLocallyControlled())//client , Locally controlled - spawn non replicated projectile, use SSR
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);
						SpawnedProjectile->bUseServerSideRewind = true;
						SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
						SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
						//SpawnedProjectile->Damage = Damage;
 
					}
					else//client , not locally controlled - spawn non replicated projectile, no SSR
					{
						SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass,
							SocketTransform.GetLocation(),
							TargetRotation,
							SpawnParams
						);

						SpawnedProjectile->bUseServerSideRewind = false;

					}
				}
			}
			else//weapon not using SSR
			{
				if (InstigatorPawn->HasAuthority())
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,
						SocketTransform.GetLocation(),
						TargetRotation,
						SpawnParams
					);

					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->Damage = HeadShotDamage;
				}
			}

 
	 
 
	}
}
