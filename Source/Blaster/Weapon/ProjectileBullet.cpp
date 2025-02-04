// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	FName PropertyName = Event.Property!=nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ANoviceCharacter* OwnerCharacter = Cast<ANoviceCharacter>(GetOwner());
	
	if (OwnerCharacter)
	{
		ANovicePlayerController* Controller = Cast<ANovicePlayerController>(OwnerCharacter->Controller);

		if (Controller)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)//if we are on the server and not locally controlled projectile fire by the simulated proxy
			{
				const float DamageToCause = (Hit.BoneName.ToString() == FString("head") || Hit.BoneName.ToString() ==  FString("PonyTail") || Hit.BoneName.ToString() ==  FString("bangs")) ? HeadShotDamage : Damage;
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, Controller, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			ANoviceCharacter* HitCharacter=Cast<ANoviceCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(HitCharacter, TraceStart, InitialVelocity,Controller->GetServerTime()-Controller->SingleTripTime );
			}
		 }

		
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

}

void AProjectileBullet::BeginPlay()
{

	Super::BeginPlay();

	/*
	 FPredictProjectilePathParams PathParams;
	 PathParams.bTraceWithChannel = true;
	 PathParams.bTraceWithCollision = true;
	 PathParams.DrawDebugTime = 5.f;
	 PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	 PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	 PathParams.MaxSimTime = 4.f;
	 PathParams.ProjectileRadius = 5.f;
	 PathParams.SimFrequency = 30.f;
	 PathParams.StartLocation = GetActorLocation();
	 PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	 PathParams.ActorsToIgnore.Add(this);

	 FPredictProjectilePathResult PathResult;
	  UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/ 
	 
	
}
