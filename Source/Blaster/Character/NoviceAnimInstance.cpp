// Fill out your copyright notice in the Description page of Project Settings.


#include "NoviceAnimInstance.h"
#include "NoviceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Weapon/WeaponTypes.h"

void UNoviceAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	NoviceCharacter = Cast<ANoviceCharacter>(TryGetPawnOwner());
}

void UNoviceAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (NoviceCharacter == nullptr)
	{
		NoviceCharacter = Cast<ANoviceCharacter>(TryGetPawnOwner());
	}
	if (NoviceCharacter == nullptr) return;

	FVector Velocity = NoviceCharacter->GetVelocity();
	Velocity.Z = 0;
	Speed = Velocity.Size();
	bIsInAir = NoviceCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = NoviceCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size()>0 ? true:false;
	bWeaponEquipped = NoviceCharacter->IsWeaponEquipped();
	bIsCrouched = NoviceCharacter->bIsCrouched;//bIsCrouched is already a variable on character class and it is also replicated
	bAiming = NoviceCharacter->IsAiming();
	TurningInPlace = NoviceCharacter->GetTurningInPlace();
	bRotateRootBone = NoviceCharacter->ShouldRotateRootBone();
	bElimmed = NoviceCharacter->IsElimmed();
	bSprinting = NoviceCharacter->GetIsSprinting();
	bIsHoldingTheFlag = NoviceCharacter->IsHoldingTheFlag();

	EquippedWeapon = NoviceCharacter->GetEquippedWeapon();
	//offset Yaw for Strafing
	FRotator AimRotation = NoviceCharacter->GetBaseAimRotation();//return the global rotation increase by +180 when controller move in right direction and decrease by 180 when move in the left direction
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(NoviceCharacter->GetVelocity());//global rotation increase by +90 when character moves right and decrese by 90 degree when move left
	
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRot,DeltaTime,7.f);//it will take the shortest path as possible so we will not go all way down to zero from positive  and negative 180 so that we will not hovering between two animations 
	//now setting Yaw
	YawOffset = DeltaRotation.Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = NoviceCharacter->GetActorRotation();

	//Now we want he delta between the two Character rotation
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 7.f);

	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = NoviceCharacter->GetAO_Yaw();
	AO_Pitch = NoviceCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && NoviceCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		NoviceCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator,OutPosition,OutRotation);//they will  become the correct  position of left hand socket
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));


		if (NoviceCharacter->IsLocallyControlled())
		{

			bLocallyControlled = true;
			//to correcting the rotation of muzzleTip toward HitTarget
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"),ERelativeTransformSpace::RTS_World);
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - NoviceCharacter->GetHitTarget()));//to look correctly
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 40.f);


			/*
			FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
			FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));//find the x forward direction of the MuzzleTip
			DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
			DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), NoviceCharacter->GetHitTarget(), FColor::Yellow);

			*/
		}


		bUseFABRIK = NoviceCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
		bool bFABRIKOverride = NoviceCharacter->IsLocallyControlled() && NoviceCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade && NoviceCharacter->bFinishSwapping;
		if (bFABRIKOverride)
		{
			bUseFABRIK = !NoviceCharacter->IsLocallyReloading() ;
		}
		bUseAimOffset= NoviceCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !NoviceCharacter->GetDisableGameplay();
		bTransformRightHand= NoviceCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !NoviceCharacter->GetDisableGameplay();
	
	}
}
 