// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "Blaster/Character/NoviceAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Weapon/HitScanWeapon.h"
#include "Blaster/Weapon/Shotgun.h"

UCombatComponent::UCombatComponent() 
{

	PrimaryComponentTick.bCanEverTick = true;
	BaseWalkSpeed = 800.f;
	AimWalkSpeed = 500.f;

	
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (NoviceCharacter) {
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (NoviceCharacter)
		{
			DefaultFOV=NoviceCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (NoviceCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	
	}
	

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
	if (NoviceCharacter && NoviceCharacter->IsLocallyControlled())
	{
		SetHUDCrosshairs(DeltaTime);

		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);
	}


}
void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (NoviceCharacter == nullptr || Weapon==nullptr ) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (Weapon->GetWeaponType() == EWeaponTypes::EWT_Flag)
	{
		bIsHoldingTheFlag = true;
		NoviceCharacter->Crouch();
		Weapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlagToLeftHand(Weapon);
		Weapon->SetOwner(NoviceCharacter);
 


	}
	else
	{
		 
		if (EquippedWeapon != nullptr && SecondaryEquippedWeapon == nullptr)
		{
			EquipSecondaryWeapon(Weapon);
		}
		else
		{
			EquipPrimaryWeapon(Weapon);
		}


		NoviceCharacter->bUseControllerRotationYaw = true;
	}
	
	
}

void UCombatComponent::SwapWeapons()
{


	if (CombatState != ECombatState::ECS_Unoccupied || NoviceCharacter==nullptr  ) return;
	NoviceCharacter->PlaySwapWeaponMontage();
	NoviceCharacter->bFinishSwapping = false;
	CombatState = ECombatState::ECS_SwapWeapon;
	AWeapon* TempWeapons = EquippedWeapon;
	EquippedWeapon = SecondaryEquippedWeapon;
	SecondaryEquippedWeapon = TempWeapons;

	if (SecondaryEquippedWeapon)
	{
		SecondaryEquippedWeapon->EnabledCustumDepth(false);

	}



}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	 
	 
	
	DroppedEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(NoviceCharacter);
	EquippedWeapon->SetInstigator(NoviceCharacter);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();

	PlayEquipSound(WeaponToEquip);
	 
	ReloadEmptyWeapon();
	
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
 

 
	SecondaryEquippedWeapon = WeaponToEquip;
	SecondaryEquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryEquippedWeapon);
	PlayEquipSound(SecondaryEquippedWeapon);


	if (SecondaryEquippedWeapon == nullptr) return;
	SecondaryEquippedWeapon->SetOwner(NoviceCharacter);
	SecondaryEquippedWeapon->SetInstigator(NoviceCharacter);

}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::PlayEquipSound(AWeapon* Weapon)
{
	if (NoviceCharacter && Weapon && Weapon->EquipSound)
	{

		UGameplayStatics::PlaySoundAtLocation(this,
			Weapon->EquipSound,
			NoviceCharacter->GetActorLocation());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedWeaponAmmo(CarriedAmmo);
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (NoviceCharacter == nullptr || NoviceCharacter->GetMesh()==nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = NoviceCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, NoviceCharacter->GetMesh());

	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (NoviceCharacter == nullptr || NoviceCharacter->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon==nullptr) return;
	
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponTypes::EWT_Pistol;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket = NoviceCharacter->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, NoviceCharacter->GetMesh());

	}
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (NoviceCharacter == nullptr || NoviceCharacter->GetMesh() == nullptr || Flag == nullptr) return;
	const USkeletalMeshSocket* HandSocket = NoviceCharacter->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(Flag, NoviceCharacter->GetMesh());

	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (NoviceCharacter == nullptr || NoviceCharacter->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	const USkeletalMeshSocket* BackSocket = NoviceCharacter->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackSocket)
	{
		BackSocket->AttachActor(ActorToAttach, NoviceCharacter->GetMesh());
	}
}

void UCombatComponent::DroppedEquippedWeapon()
{
	if (EquippedWeapon) {
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState==ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		HandleReload();
		ServerReload();
		bLocallyReloading = true;
		

	}
}

void UCombatComponent::FinishReloading()
{
	if (NoviceCharacter == nullptr) return;

	bLocallyReloading = false;
	if (NoviceCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	} 
	if (bFireButtonPressed)
	{
		Fire();
	}
	
}

void UCombatComponent::FinishSwap()
{
	if (NoviceCharacter && NoviceCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}

	if (NoviceCharacter )
	{
		NoviceCharacter->bFinishSwapping = true;
	}
 
	if (SecondaryEquippedWeapon)
	{
		SecondaryEquippedWeapon->EnabledCustumDepth(true);

	}
}

void UCombatComponent::FinishSwapAttachWeapon()
{

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipSound(EquippedWeapon);

	SecondaryEquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryEquippedWeapon);

}

void UCombatComponent::FinishGrenadeThrow()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon)
	{
		EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()//this is called on all the machine straight through the animation Blueprint
{
	ShowAttachedGrenade(false);

	if (NoviceCharacter && NoviceCharacter->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);

	}
}


void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (NoviceCharacter && GrenadeClass && NoviceCharacter->GetAttachedGrenade())
	{
		const FVector StartingLocation = NoviceCharacter->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget =Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = NoviceCharacter;
		SpawnParams.Instigator = NoviceCharacter;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
			);
		}

	}
}


void UCombatComponent::HandleReload()
{
	if(NoviceCharacter) NoviceCharacter->PlayReloadMontage();
	
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int Least = FMath::Min(RoomInMag, AmountCarried);

		return FMath::Clamp(RoomInMag, 0, Least);// if roomInMag is 5 and carried ammo is 2 so  we only reload 2 ammo
	}
	return 0;
}

void UCombatComponent::ServerReload_Implementation()
{
	if (NoviceCharacter == nullptr || EquippedWeapon==nullptr) return;

	
	CombatState = ECombatState::ECS_Reloading;//Once it is set here it is replicated down to all the clients and RepNotify will be called for CombatState
	if(!NoviceCharacter->IsLocallyControlled()) HandleReload();
	
}

void UCombatComponent::UpdateAmmoValues()
{
	if (NoviceCharacter == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedWeaponAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (NoviceCharacter == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedWeaponAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	//doing this on the server and for client it will be called in onRep_Ammo in the weapon class , and for carried ammo called in the onRep_CarriedAmmo
	if (EquippedWeapon->IsFull() || CarriedAmmo==0)
	{
		JumpToShotgunEnd();
	}
	
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	PlayerController = PlayerController == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : PlayerController;
	if (PlayerController) {
		PlayerController->SetHUDCarriedGrenade(Grenades);
	}
}

void UCombatComponent::OnRep_bIsHoldingTheFlag()
{
	if (bIsHoldingTheFlag && NoviceCharacter && NoviceCharacter->IsLocallyControlled())
	{
		NoviceCharacter->Crouch();
	}
}

bool UCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon!=nullptr && SecondaryEquippedWeapon!=nullptr);
}



void UCombatComponent::OnRep_CombatState()
{

	switch (CombatState)
	{

	case ECombatState::ECS_Reloading:
		if(NoviceCharacter && !NoviceCharacter->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
			if (bFireButtonPressed)
			{
				Fire();
			}
			
			break;
	case ECombatState::ECS_ThrowingGrenade:
		 if(NoviceCharacter && !NoviceCharacter->IsLocallyControlled())
		 {
			 NoviceCharacter->PlayThrowGrenadeMontage();
			 AttachActorToLeftHand(EquippedWeapon);
			 ShowAttachedGrenade(true);
		 }
	 break;

	case ECombatState::ECS_SwapWeapon:
		if (NoviceCharacter && !NoviceCharacter->IsLocallyControlled())
		{
			NoviceCharacter->PlaySwapWeaponMontage();
		}
		break;
	default:
		break;
	}
}


void UCombatComponent::OnRep_EquippedWeapon()
{

	if (EquippedWeapon && NoviceCharacter)
	{
		
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);//making sure that physics properties set first before attaching to the client hands
		AttachActorToRightHand(EquippedWeapon);

		PlayEquipSound(EquippedWeapon);
		NoviceCharacter->bUseControllerRotationYaw = true;

		EquippedWeapon->EnabledCustumDepth(false);
		EquippedWeapon->SetHUDAmmo();
		
	}


}

void UCombatComponent::OnRep_SecondaryEquippedWeapon()
{
	if (SecondaryEquippedWeapon && NoviceCharacter)
	{
		SecondaryEquippedWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackpack(SecondaryEquippedWeapon);

		PlayEquipSound(SecondaryEquippedWeapon);

	
		
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}

	
}

void UCombatComponent::ShotgunShellReload()
{
	if (NoviceCharacter && NoviceCharacter->HasAuthority()) 
	{
		UpdateShotgunAmmoValues();
	}
	
}

void UCombatComponent::JumpToShotgunEnd()
{

		//jump to shot gun end section

		UAnimInstance* AnimInstance = NoviceCharacter->GetMesh()->GetAnimInstance();
		if (AnimInstance && NoviceCharacter->GetReloadMontage()) {
			AnimInstance->Montage_JumpToSection(FName("ShotgunReloadEnd"));

		}
	
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;

		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->FireType)
			{
			case EFireTypes::EFT_Projectile:
				FireProjectileWeapon();
				break;

			case EFireTypes::EFT_HitScan:
				FireHitScanWeapon();
				break;

			case EFireTypes::EFT_Shotgun:
				FireShotgun();
				break;
			}
		}
		StartFireTimer();//it will be call again and again as fire Function called from FireTimerFinished and Finished Fire Timer is called from StartFireTimer by its timer manager

	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && NoviceCharacter)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		
		if(!NoviceCharacter->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}

}

void UCombatComponent::FireHitScanWeapon()
{

	if (EquippedWeapon && NoviceCharacter)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!NoviceCharacter->HasAuthority()) LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);

	if (Shotgun && NoviceCharacter)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (!NoviceCharacter->HasAuthority()) LocalShotgunFire(HitTargets);
		ServerShotgunFire(HitTargets,EquippedWeapon->FireDelay);
	}
	
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)//Validate function for stopping player to hack the fire delay
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay,0.001f);
		return bNearlyEqual;
	}
	return true;
}
 

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (NoviceCharacter && NoviceCharacter->IsLocallyControlled() && !NoviceCharacter->HasAuthority()) return;//fire not perform on the clients machine we are performing locally on the clients 
	LocalFire(TraceHitTarget);//play on those machines character which are server and not locally controlled on all other machines except the owing one(for lag)
	
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	
	if (EquippedWeapon == nullptr) return;

	if (NoviceCharacter && CombatState == ECombatState::ECS_Unoccupied)
	{
		NoviceCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
	
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || NoviceCharacter==nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		NoviceCharacter->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (NoviceCharacter && NoviceCharacter->IsLocallyControlled() && !NoviceCharacter->HasAuthority()) return;//fire not perform on the clients machine we are performing locally on the clients 
	LocalShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)//Validate function for stopping player to hack the fire delay
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || NoviceCharacter == nullptr) return;
	
	NoviceCharacter->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	); //continuously calling FireTimerFinished at constant rate of FireDelay time
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr || NoviceCharacter == nullptr) return;

	bCanFire = true;//again setting it true
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr ) return false;

	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponTypes::EWT_Shotgun) return true;
	if (bLocallyReloading) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState==ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDCarriedWeaponAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponTypes::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_AssaultRifle,StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponTypes::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryEquippedWeapon);
	DOREPLIFETIME(UCombatComponent,bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, bIsHoldingTheFlag);
	DOREPLIFETIME_CONDITION(UCombatComponent,CarriedAmmo,COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, Grenades);
}


void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);//finding the centre for crosshair these are x and y coordinate in screen space and we want in world space
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (NoviceCharacter)
		{
			float DistanceToCharacter = (NoviceCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 90.f);

		}
		FVector End = Start + CrosshairWorldDirection * Trace_Lenght;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		//DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			Package.CrosshairsColor = FLinearColor::Red;
		}
		else {

			Package.CrosshairsColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (NoviceCharacter == nullptr || NoviceCharacter->Controller == nullptr) return;

	PlayerController = PlayerController == nullptr ? Cast<ANovicePlayerController>(NoviceCharacter->Controller) : PlayerController;
	if (PlayerController)
	{
		HUD = HUD == nullptr ? Cast<ANoviceHUD>(PlayerController->GetHUD()) : HUD;
		if (HUD)
		{
			//Weapons have their  own  different Crosshair so we are selecting the crosshairs texture from weapon blueprint and then assigning those texture to our HUD from Setter with the help of FHUDPackage Struct that we created in ANoviceHUD. Doing all these in combat component in this function SettingHUD

			if (EquippedWeapon) {


				Package.CrosshairsCentre = EquippedWeapon->CrosshairCentre;
				Package.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
				Package.CrosshairsTop = EquippedWeapon->CrosshairTop;
				Package.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				Package.CrosshairsRight = EquippedWeapon->CrosshairRight;


			}
			else {

				Package.CrosshairsCentre = nullptr;
				Package.CrosshairsBottom = nullptr;
				Package.CrosshairsTop = nullptr;
				Package.CrosshairsLeft = nullptr;
				Package.CrosshairsRight = nullptr;


			}
			//calculate the crosshairs spread
			//we want to map our speed from [0,800] -> [0,1]
			float NoviceCharacterSpeed = NoviceCharacter->bIsCrouched ? NoviceCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched : NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed;
			FVector2D WalkSpeedRange(0.f, NoviceCharacterSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);

			FVector Velocity = NoviceCharacter->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());//this function is mapping the velocity between 0 to 1
			if (NoviceCharacter->GetCharacterMovement()->IsFalling())
			{
				//like to interpolate the crosshairs slowly
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);//when we reach the ground it will become zero
			}
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			Package.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(Package);
		}

	}

}

void UCombatComponent::ThrowGrenade()
{

	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon==nullptr) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (NoviceCharacter)
	{
		NoviceCharacter->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);//locally
	}
	if (NoviceCharacter && !NoviceCharacter->HasAuthority())
	{
		ServerThrowGrenade();

	}
	if (NoviceCharacter && NoviceCharacter->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}

	
	
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (NoviceCharacter && NoviceCharacter->GetAttachedGrenade())
	{
		NoviceCharacter->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}


void UCombatComponent::PickupAmmo(EWeaponTypes WeaponTypes, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponTypes))
	{
		CarriedAmmoMap[WeaponTypes] = FMath::Clamp(CarriedAmmoMap[WeaponTypes]+AmmoAmount,0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && WeaponTypes == EquippedWeapon->GetWeaponType())
	{
		Reload();
	}

}

void UCombatComponent::ServerThrowGrenade_Implementation()
{

	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;

	if (NoviceCharacter)
	{
		NoviceCharacter->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades-1,0,MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) { return; }
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	/*
		FVector Start = NoviceCharacter->GetFollowCamera()->GetComponentLocation();
	const USkeletalMeshSocket* ADSSocket = EquippedWeapon->GetWeaponMesh()->GetSocketByName(FName("ADS"));
	FTransform SocketTransform = ADSSocket->GetSocketTransform(EquippedWeapon->GetWeaponMesh());
	const FVector TargetADS = SocketTransform.GetLocation();
	*/

	if (bAiming)
	{

		
		//FVector ADSLocation = FMath::VInterpTo(NoviceCharacter->GetActorLocation(), TargetADS, DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
		//NoviceCharacter->GetFollowCamera()->SetWorldLocation(ADSLocation);

		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		//FVector ADSLocation = FMath::VInterpTo(TargetADS, Start, DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
		//NoviceCharacter->GetFollowCamera()->SetWorldLocation(ADSLocation);
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (NoviceCharacter && NoviceCharacter->GetFollowCamera())
	{
		NoviceCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}




void UCombatComponent::SetAiming(bool bIsAiming)
{

	if (NoviceCharacter == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming;

	ServerSetAiming(bIsAiming);//calling it from the server means bIsAiming will be set on the server ,but the invoke from the client it will be executed on the server as well
	if (NoviceCharacter) {
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	if (NoviceCharacter->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponTypes::EWT_SniperRifle)
	{
		NoviceCharacter->ShowSniperScopeWidget(bIsAiming);
	}

	if(NoviceCharacter->IsLocallyControlled()) bAimButtonPressed = bIsAiming;

}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (NoviceCharacter) {
		NoviceCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}


void UCombatComponent::OnRep_Aiming()
{
	if (NoviceCharacter && NoviceCharacter->IsLocallyControlled())//called on the owning machine
	{
		bAiming = bAimButtonPressed;
	}
}

