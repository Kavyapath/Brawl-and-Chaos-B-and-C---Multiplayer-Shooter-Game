// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/Enums/TurningInPlace.h"
#include "InputActionValue.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Blaster/Enums/CombatStates.h"
#include "Components/TimelineComponent.h"
#include "NoviceCharacter.generated.h"


//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FReverseTimeDelegate, bool, bReverseTime);

class UInputMappingContext;
class UInputAction;
class AWeapon;
class UBoxComponent;
UCLASS()
class BLASTER_API ANoviceCharacter : public ACharacter ,public IInteractWithCrosshairsInterface 
{
	GENERATED_BODY()

public:
	
	ANoviceCharacter();
	virtual void Tick(float DeltaTime) override;
	void RotateInPlace(float DeltaTime);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();
	void PlayReloadMontage();
	void Elim();
	void DropOrDestroyWeapons();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void PlayThrowGrenadeMontage();
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();

	void UpdateHUDShield();

	void UpdateHUDAmmo();


	virtual void OnRep_ReplicatedMovement() override;// its replicated variable is ReplicatedMovement
	
	//Hit boxes for server side rewind
	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxes;
protected:
	
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void PossessedBy(AController* NewController) override;
	

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Equip();
	void Crouching();
	void Aim();
	void AimRelease();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void StartSprinting();
	void StopSprinting();
	void ReloadButtonPressed();
	void GrenadeButtonPressed();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	//poll for any relevant classes and initialize our HUD
	void PollInit();
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* GrenadeThrowAction;

	UPROPERTY(VisibleAnywhere,Category=Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;



	/*
	Grenade Mesh
	*/
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* AttachedGrenade ;

	UPROPERTY(EditAnywhere)
	float RotationRate = 15;

	UPROPERTY(EditAnywhere)
	float SprintSpeed = 1000.f;

	UPROPERTY()
	class ANovicePlayerController* NovicePlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing =OnRep_OverlappingWeapon)
	 AWeapon* OverlappingWeapon;
	//this is the conviction of the function to declare it

	 /*
	 Components
	 */
	 UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess="true"))
	 class UCombatComponent* Combat;

	 UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	 class ULagCompensationComponent* LagCompensation;

	 UPROPERTY(VisibleAnywhere)
	 class UBuffComponent* Buff;
	 /*
	 animations
	 */
	 UPROPERTY(EditAnywhere,Category=Combat)
	 class UAnimMontage* FireWeaponMontage;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 class UAnimMontage* HitReactMontage;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 class UAnimMontage* ElimMontage;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 class UAnimMontage* ReloadMontage;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 class UAnimMontage* GrenadeThrowMontage;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);//RepNotify functions its get call perpetually when variable is replicated

	UFUNCTION(Server,Reliable) //this is an RPC(Remote procedure calls that will be  call on one machine and can be executed on another machine) Reliable means this function will be executed guaranteed 
	void ServerEquipButtonPressed();



	void Sprint(bool CanSprint);
	 
	UFUNCTION(Server, Reliable) 
		void ServerSprint(bool CanSprint);

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;

	UPROPERTY(EditAnywhere)
	float TurnThreshold = 0.8f;

	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();


	UPROPERTY(Replicated)
	bool IsSprinting;

	bool FireOn;
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* Niagara;


	UPROPERTY(EditAnywhere,Category="Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing=OnRep_Health,VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;
	// we are replicating the health So every time the health is replicated down to the client
	
	/*
	shield
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	bool bElimmed = false;
	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelayTime = 3.f;


	void ElimTimerFinished();

	/*
	DissolveEffect
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;//dynamic delegate

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	//Dynamic instance that we can change at run time
	UPROPERTY(VisibleAnywhere,Category= Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;//to store the dynamic istance that we create

	//Material instance set on the Blueprint , used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;//set this on blueprint

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceEye;//to store the dynamic istance that we create

	//Material instance set on the Blueprint , used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstanceEye;//set this on blueprint

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceTeeth;//to store the dynamic istance that we create

	//Material instance set on the Blueprint , used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstanceTeeth;//set this on blueprint

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceHair;//to store the dynamic istance that we create

	//Material instance set on the Blueprint , used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstanceHair;//set this on blueprint

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceCloth;//to store the dynamic istance that we create

	//Material instance set on the Blueprint , used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstanceCloth;//set this on blueprint



	/*
	Elim Bot
	*/
	UPROPERTY(EditAnywhere, Category = Elim)
	UParticleSystem* ElimBotEffect;
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = Elim)
	class USoundCue* ElimBotSound;
	UPROPERTY()
	class ANovicePlayerState* NovicePlayerState;
	/*
	Default Weapon
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	void SpawnDefaultWeapon();
public:	
	void SetOverlappingWeapon(AWeapon* Weapon);
	 bool IsWeaponEquipped();
	 bool IsAiming();

	 FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	 FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	 AWeapon* GetEquippedWeapon();
	 FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	 FVector GetHitTarget() const ;
	 FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	 FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	 FORCEINLINE bool IsElimmed() const  { return bElimmed; }
	 FORCEINLINE float GetHealth() const { return Health; }
	 FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	 FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	 FORCEINLINE bool GetIsSprinting() const  { return IsSprinting;
	 }
	 ECombatState GetCombatState() const;
	 FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
	 FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay;
	 }
	 FORCEINLINE UAnimMontage* GetReloadMontage() const {
		 return ReloadMontage;
	 }
	 FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	 FORCEINLINE UBuffComponent* GetBuffComponent() const { return Buff; }
	 FORCEINLINE float GetMaxShield() const { return MaxShield; }
	 FORCEINLINE float GetShield() const { return Shield; }
	 FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	 bool IsLocallyReloading();
	 FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensation; }
};
