// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Blaster/BlasterComponent/BuffComponent.h"

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(OtherActor);
	if (NoviceCharacter)
	{
		UBuffComponent* Buff = NoviceCharacter->GetBuffComponent();
		if (Buff)
		{
			Buff->BuffSpeed(BaseSpeedBuff,CrouchSpeedBuff,SpeedBuffTime);
		}
	}
	Destroy();
}
