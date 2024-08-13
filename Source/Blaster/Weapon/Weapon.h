 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_InitialState UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_Max UMETA(DisplayName = "DefaultMax")



};

UENUM(BlueprintType)
enum class EFireTypes :uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan Weapon"),
	EFT_Projectile UMETA(DisplayName = " Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = " Shotgun Weapon"),

	EWS_Max UMETA(DisplayName = "DefaultMax")
};

UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;
	virtual void OnRep_Owner() override;
	virtual void Fire(const FVector& HitTarget);
	void Dropped();
	void SetHUDAmmo();
	void AddAmmo(int32 AmmoToAdd);
	FVector TraceEndWithScatter(const FVector& HitTarget);
protected:

	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void OnWeaponStateSet();

	void OnDropped();

	void OnEquipped();
	void OnEquippedSecondary();

	/*
Trace end with scatter

*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;


	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;


private:
	UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	 USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	 class USphereComponent* AreaSphere;

	 UPROPERTY(ReplicatedUsing=OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	 EWeaponState WeaponState;

	 UFUNCTION()
	 void OnRep_WeaponState();

	 UPROPERTY(VisibleAnywhere,Category="Weapon Properties")
	 class UWidgetComponent* PickupWidget;

	 UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	 class UAnimationAsset* FireAnimation;

	 UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	 TSubclassOf<class ACasing> CasingClass;


	 UPROPERTY(EditAnywhere)
	 int32 Ammo;

	 /*
	 	 UFUNCTION()
	 void OnRep_Ammo();

	 */

	 
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);



	 UFUNCTION(Client, Reliable)
	 void ClientAddAmmo(int32 AmmoToAdd);
	
	 
	 void SpendRound();

	 UPROPERTY(EditAnywhere)
	 int32 MagCapacity;

	 //The number of unprocessed server request for Ammo.
	 //Incremented in SpendRound, decremented in ClientUpdateAmmo.
	 int32 Sequence=0;
	 /*
 Zoomed FOV While Aiming
 */
	 UPROPERTY(EditAnywhere)
	 float ZoomedFOV = 30.f;

	 UPROPERTY(EditAnywhere)
	 float ZoomInterpSpeed = 20.f;

	 UPROPERTY()
	 class ANoviceCharacter* NoviceOwnerCharacter;

	 UPROPERTY()
	 class ANovicePlayerController* NoviceOwnerController;

	 UPROPERTY(EditAnywhere)
	 EWeaponTypes WeaponType;


public:
	/*
	 Texture for the weapon crosshair
	 */
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairCentre;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairLeft;

	/*
	Automatic Fire
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .10f;//this will also become as rate of fire for weapons
	
	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;
	/*
	
	Enabling and sisabling custum depth*/

	void EnabledCustumDepth(bool bEnabled);

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireTypes FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

public:
	 void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();
	bool IsFull();
	FORCEINLINE EWeaponTypes GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

};
