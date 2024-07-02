// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/NoviceHUD.h"
#include "Blaster/Enums/CombatStates.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"




class AWeapon;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;
	friend class ANoviceCharacter;//giving full access of this class to NoviceCharacter Class

	void EquipWeapon(AWeapon* Weapon);
	void ReloadEmptyWeapon();
	void PlayEquipSound();
	void UpdateCarriedAmmo();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void DroppedEquippedWeapon();
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishGrenadeThrow();

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	void JumpToShotgunEnd();
protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bIsAiming);//RPC

	UFUNCTION(Server, Reliable)
	void ServerReload();//RPC

	void HandleReload();

	int32 AmountToReload();


	UFUNCTION()
	void OnRep_EquippedWeapon();



	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);
	
	void ThrowGrenade();
	UFUNCTION(Server,Reliable)
	void ServerThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);
	void ShowAttachedGrenade(bool bShowGrenade);

	void PickupAmmo(EWeaponTypes WeaponTypes, int32 AmmoAmount);

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
private:
	UPROPERTY()
	class ANoviceCharacter* NoviceCharacter;
	UPROPERTY()
	class ANovicePlayerController* PlayerController;
	UPROPERTY()
	class ANoviceHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/*
	HUD and Crosshairs
	*/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;
	FHUDPackage Package;
	/*
	Aiming and FOV
	*/

	//Field of view when not aiming; set to camera;s base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere,Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);
	
	
	void AimDownSights();

	
	void StopsAimDownSights();
	/*
	Automatic Fire
	*/

	FTimerHandle FireTimer;


		
	bool bCanFire=true;//player does not call the fire function on spamming mouse button agin it will be set true again when FireTimerFinished() function finish so again fire function will be called after a time delay of FireDelay variable,To maintain a constant rate of fire or the weapons

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	//Carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	 void OnRep_CarriedAmmo();


	 TMap<EWeaponTypes, int32> CarriedAmmoMap;
	 UPROPERTY(EditAnywhere)
	 int32 StartingARAmmo = 30;

	 UPROPERTY(EditAnywhere)
	 int32 StartingRocketAmmo = 0;

	 UPROPERTY(EditAnywhere)
	 int32 StartingPistolAmmo = 0;

	 UPROPERTY(EditAnywhere)
	 int32 StartingSMGAmmo = 0;

	 UPROPERTY(EditAnywhere)
	 int32 StartingSniperAmmo = 0;

	 UPROPERTY(EditAnywhere)
	 int32 StartingShotgunAmmo = 0;

	 UPROPERTY(EditAnywhere)
	 int32 StartingGrenadeLauncherAmmo = 0;

	 void InitializeCarriedAmmo();

	 UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	 ECombatState CombatState=ECombatState::ECS_Unoccupied;

	 UFUNCTION()
	 void OnRep_CombatState();

		
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing=OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 5;

	void UpdateHUDGrenades();

public:
	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
}; 
