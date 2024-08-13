// Fill out your copyright notice in the Description page of Project Settings.


#include "NoviceCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Blaster/PlayerState/NovicePlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Components/BoxComponent.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
 
ANoviceCharacter::ANoviceCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom,USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);
	

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);//this is how components are replicated

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComp"));
	Buff->SetIsReplicated(true);


	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));


	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	Niagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Niagara Effect"));
	Niagara->SetupAttachment(FollowCamera);

	/*
	Hit boxes for server side rewind
	*/

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	head->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);


	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);


	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);


	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("BackpackSocket"));
	backpack->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);


	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);
	
	
	
}


void ANoviceCharacter::OnRep_ReplicatedMovement()
{

	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();

	TimeSinceLastMovementReplication = 0.f;
}

void ANoviceCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this,&ANoviceCharacter::ReceiveDamage);
	}
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}
void ANoviceCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


void ANoviceCharacter::UpdateHUDHealth()
{
	NovicePlayerController = NovicePlayerController == nullptr ? Cast<ANovicePlayerController>(GetController()) : NovicePlayerController;
	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ANoviceCharacter::UpdateHUDShield()
{
	NovicePlayerController = NovicePlayerController == nullptr ? Cast<ANovicePlayerController>(GetController()) : NovicePlayerController;
	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDShield(Shield, MaxShield);
	}
}

void ANoviceCharacter::UpdateHUDAmmo()
{
	NovicePlayerController = NovicePlayerController == nullptr ? Cast<ANovicePlayerController>(GetController()) : NovicePlayerController;
	if (NovicePlayerController &&Combat && Combat->EquippedWeapon)
	{
		NovicePlayerController->SetHUDCarriedWeaponAmmo(Combat->CarriedAmmo);
		NovicePlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ANoviceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);

	HideCameraIfCharacterClose();
	PollInit();
	


}

void ANoviceCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled()) // Autonomus proxy and Authority are greater than simuated proxy
	{
		AimOffset(DeltaTime);//calling AimOffset every frame
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		//aslo calculate pitch every frame
		CalculateAO_Pitch();
	}

}




void ANoviceCharacter::Move(const FInputActionValue& Value)
{
	//if (Combat->GetCombatState() == ECombatState::ECS_Reloading) return;
	if (bDisableGameplay) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();
	//controller(Mouse) can only rotate in XY axis remenber that  (Rotation along Z Axis) is Controller is at (0,0,0) and Rotate by 45 Degree in XY plane now Controller Rotation becomes (45,45,0)
   //FRotationMatrix(YawRotation) Simple means Rotation Maxtrix formula Rotation Along ZAxis(YawRotation) Go And check the formula(According to unreal engine gizmo the XAxis Points to the forward direction)
	const FRotator ControllerDirection = GetControlRotation();
	const FRotator YawRotation(0.f, ControllerDirection.Yaw, 0.f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);//we wsant to find the forward direction when we will press w Xcos(theta)
	AddMovementInput(ForwardDirection, MovementVector.Y);

	//Finding Which way is right
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y); //controller(Mouse) can only rotate in XY axis remenber that  (Rotation along Z Axis) is Controller is at (0,0,0) and Rotate by 45 Degree in XY plane now Controller Rotation becomes (45,45,0)
	//FRotationMatrix(YawRotation) Simple means Rotation Maxtrix formula Rotation Along ZAxis(YawRotation) Go And check the formula
	AddMovementInput(RightDirection, MovementVector.X);

}

void ANoviceCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D AxisValue = Value.Get<FVector2D>();
	if (GetController()) {
		AddControllerPitchInput(AxisValue.Y * RotationRate * GetWorld()->GetDeltaSeconds());
		AddControllerYawInput(AxisValue.X * RotationRate * GetWorld()->GetDeltaSeconds());
	}
}

void ANoviceCharacter::Equip()
{
	if (bDisableGameplay) return;
	if (Combat )
	{
		ServerEquipButtonPressed();
		
	}

}

void ANoviceCharacter::Crouching()
{
	if (bDisableGameplay) return;
	if (bIsCrouched) 
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}


}

void ANoviceCharacter::Aim()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
		//Combat->AimDownSights();
		
	}
}

void ANoviceCharacter::AimRelease()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
		//Combat->StopsAimDownSights();
		
	}
}

float ANoviceCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0;
	return Velocity.Size();
}

void ANoviceCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;

	float Speed = CalculateSpeed();
	 bool bIsInAir = GetCharacterMovement()->IsFalling();
	 if (Speed == 0 && !bIsInAir)//standing still and not jumping
	 {
		 bRotateRootBone = true;
		 FRotator FinalAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw,0.f);
		 FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(FinalAimRotation,StartingAimRotation);
		 AO_Yaw = Delta.Yaw;
		 if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		 {
			 InterpAO_Yaw = AO_Yaw;
		 }
		 bUseControllerRotationYaw = true;
		 TurnInPlace(DeltaTime);
	 }
	 if (Speed > 0.f || bIsInAir)//running or jumping
	 {
		 bRotateRootBone = false;
		 StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//storing starting aim rotation value when player is  moving or is in air
		 AO_Yaw = 0.f;
		 bUseControllerRotationYaw = true;
		 TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	 }

	 CalculateAO_Pitch();
}

void ANoviceCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map the pitch from [270,360) to [-90,0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);

	}
}

void ANoviceCharacter::SimProxiesTurn()
{
	if(Combat==nullptr || Combat->EquippedWeapon==nullptr ) return;
	bRotateRootBone = false; 
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();

	ProxyYaw=UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation,ProxyRotationLastFrame).Yaw;
	
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else {
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ANoviceCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}

void ANoviceCharacter::FireButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat->EquippedWeapon == nullptr) return;
	
	IsSprinting = false;
	if (Combat) {
		Combat->FireButtonPressed(true);
		FireOn = true;
		
		
	}
	
}

void ANoviceCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat->EquippedWeapon == nullptr) return;
	
	if (Combat) {
		Combat->FireButtonPressed(false);
		FireOn = false;

	}
}

void ANoviceCharacter::TurnInPlace(float DeltaTime)
{
	
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)//if we are turning
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 40.f);
		AO_Yaw = InterpAO_Yaw;//this will be going to update continuously as  we calling this from AimOffset which is called in tick function every frame
		if (FMath::Abs(AO_Yaw) < 20.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}

}

void ANoviceCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//for hiding the weapon mesh
		}
		
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//for hiding the weapon mesh
		}
	}
}

void ANoviceCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}


void ANoviceCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
		
	}
}


void ANoviceCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ANoviceCharacter::OnRep_Health(float LastHealth)
{
	//Rep_Notifies only call on the client
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
	
}


void ANoviceCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
	if (DynamicDissolveMaterialInstanceEye)
	{
		DynamicDissolveMaterialInstanceEye->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
	if (DynamicDissolveMaterialInstanceTeeth)
	{
		DynamicDissolveMaterialInstanceTeeth->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
	if (DynamicDissolveMaterialInstanceHair)
	{
		DynamicDissolveMaterialInstanceHair->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
	if (DynamicDissolveMaterialInstanceCloth)
	{
		DynamicDissolveMaterialInstanceCloth->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ANoviceCharacter::StartDissolve()
{
	//it is going to start our time line
	DissolveTrack.BindDynamic(this, &ANoviceCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve,DissolveTrack);//all this does is it sets up our time line to use this dissolve curve and associate that curve with our dissolve track which is our call back
		DissolveTimeline->Play();
	
	}
}

void ANoviceCharacter::SpawnDefaultWeapon()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));//we return null if we arenot on the server
	UWorld* World = GetWorld();
	if (GameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon=World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

void ANoviceCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	//if we are on the server the this function passes true on the character that actually being controlled
	if (IsLocallyControlled()) {

		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}

}

bool ANoviceCharacter::IsWeaponEquipped()
{
	
	return (Combat && Combat->EquippedWeapon);
}

bool ANoviceCharacter::IsAiming()
{
	return(Combat && Combat->bAiming);
}

AWeapon* ANoviceCharacter::GetEquippedWeapon()
{
	if(Combat==nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ANoviceCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState ANoviceCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_Max;
	return Combat->CombatState;
}

bool ANoviceCharacter::IsLocallyReloading()
{
	if(Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}



// Called to bind functionality to input
void ANoviceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		//crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ThisClass::Crouching);
		//equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ThisClass::Equip);
		//Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::Aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimRelease);
		//Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started , this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);

		//Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::StartSprinting);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::StopSprinting);
		//Relaod
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ThisClass::ReloadButtonPressed);
		//throw grenade
		EnhancedInputComponent->BindAction(GrenadeThrowAction, ETriggerEvent::Triggered, this, &ThisClass::GrenadeButtonPressed);


	
		
	}

}

void ANoviceCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	//Register the variable that you want to replicate here in this function
	DOREPLIFETIME_CONDITION(ANoviceCharacter, OverlappingWeapon,COND_OwnerOnly);//macro only replicate data to the client which is owning the pawn and  share to the server not to other clients
	DOREPLIFETIME(ANoviceCharacter, Health);
	DOREPLIFETIME(ANoviceCharacter, IsSprinting);
	DOREPLIFETIME(ANoviceCharacter, bDisableGameplay);
	DOREPLIFETIME(ANoviceCharacter, Shield);

}

void ANoviceCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->NoviceCharacter = this;
	}
	if (Buff)
	{
		Buff->NoviceCharacter = this;
		Buff->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpSpeed(GetCharacterMovement()->JumpZVelocity);
	
	}
	if (LagCompensation)
	{
		LagCompensation->NoviceCharacter = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ANovicePlayerController>(Controller);
		}
	}
}

void ANoviceCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon==nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);

	}
}

void ANoviceCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(ElimMontage);
	
		 
	}
}

void ANoviceCharacter::PlayReloadMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage) {
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{ 
			case EWeaponTypes::EWT_AssaultRifle:
			SectionName=FName("Rifle");
			break;

			case EWeaponTypes::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;

			case EWeaponTypes::EWT_Pistol:
			SectionName = FName("Pistol");
			break;

			case EWeaponTypes::EWT_SMG:
			SectionName = FName("Pistol");
			break;

			case EWeaponTypes::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;

			case EWeaponTypes::EWT_SniperRifle:
			SectionName = FName("Sniper");
			break;

			case EWeaponTypes::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;

			default:
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName,ReloadMontage);


	}

}

void ANoviceCharacter::Elim()
{

	DropOrDestroyWeapons();
	MulticastElim();
	

	GetWorldTimerManager().SetTimer(ElimTimer,this, &ANoviceCharacter::ElimTimerFinished, ElimDelayTime);
}

void ANoviceCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryEquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryEquippedWeapon);
		}


	}
}

void ANoviceCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;

	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();//if we have default weapon with us destroy the weapon
	}
	else {
		Weapon->Dropped();
	}
}

void ANoviceCharacter::PlayThrowGrenadeMontage()
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeThrowMontage) {
		AnimInstance->Montage_Play(GrenadeThrowMontage);
		FName SectionName;
		SectionName = bIsCrouched ? FName("SittingThrow") : FName("StandingThrow");
		AnimInstance->Montage_JumpToSection(SectionName,GrenadeThrowMontage);
	}
}

void ANoviceCharacter::MulticastElim_Implementation()
{

	if (NovicePlayerController)
	{
		NovicePlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();
	//Start Dissolve effect
	if (DissolveMaterialInstance) {
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
		
	
	}
	if (DissolveMaterialInstanceEye) {
		DynamicDissolveMaterialInstanceEye = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceEye, this);
		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstanceEye);
		DynamicDissolveMaterialInstanceEye->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstanceEye->SetScalarParameterValue(TEXT("Glow"), 200.f);


	}
	if (DissolveMaterialInstanceTeeth) {
		DynamicDissolveMaterialInstanceTeeth = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceTeeth, this);
		GetMesh()->SetMaterial(2, DynamicDissolveMaterialInstanceTeeth);
		DynamicDissolveMaterialInstanceTeeth->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstanceTeeth->SetScalarParameterValue(TEXT("Glow"), 200.f);


	}
	if (DissolveMaterialInstanceHair) {
		DynamicDissolveMaterialInstanceHair = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceHair, this);
		GetMesh()->SetMaterial(3, DynamicDissolveMaterialInstanceHair);
		DynamicDissolveMaterialInstanceHair->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstanceHair->SetScalarParameterValue(TEXT("Glow"), 200.f);


	}
	if (DissolveMaterialInstanceCloth) {
		DynamicDissolveMaterialInstanceCloth = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceCloth, this);
		GetMesh()->SetMaterial(4, DynamicDissolveMaterialInstanceCloth);
		DynamicDissolveMaterialInstanceCloth->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstanceCloth->SetScalarParameterValue(TEXT("Glow"), 200.f);


	}
	StartDissolve();

	//Disable Character movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	bDisableGameplay = true;
	if (Combat) {
		Combat->FireButtonPressed(false);
	}


	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn ElimBot

	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation() + FVector(0.f, 0.f,200.f));
		ElimBotComponent=UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint, 
			GetActorRotation());
	}
	if (ElimBotSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponTypes::EWT_SniperRifle;
	if (bHideSniperScope)//player scope is open and he die then shut off his scope reverse animation
	{
		ShowSniperScopeWidget(false);
	}
}

void ANoviceCharacter::ElimTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this,Controller);
		
	}

	
}
void ANoviceCharacter::Destroyed()
{
	Super::Destroyed();
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();//as Character is replicated variable and we are destorying it in the game mode
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}


void ANoviceCharacter::PlayHitReactMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
	
}

void ANoviceCharacter::Sprint(bool CanSprint)
{
	IsSprinting = CanSprint;
	ServerSprint(CanSprint);
	if (Combat)
	{
		GetCharacterMovement()->MaxWalkSpeed = CanSprint ? SprintSpeed : Combat->BaseWalkSpeed;
	}

	//GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ANoviceCharacter::ServerSprint_Implementation(bool CanSprint)
{
	IsSprinting = CanSprint;

	if (Combat)
	{
		GetCharacterMovement()->MaxWalkSpeed = CanSprint ? SprintSpeed : Combat->BaseWalkSpeed;
	}

	//GetCharacterMovement()->bOrientRotationToMovement = true;
}






void ANoviceCharacter::StartSprinting()
{
	if (FireOn || bDisableGameplay) return;
	FVector Zero(0.f, 0.f, 0.f);
	if (GetCharacterMovement()->Velocity == Zero) {
		return;

	}
	Niagara->Activate();
	Sprint(true);
	
	
}

void ANoviceCharacter::StopSprinting()
{
	if (bDisableGameplay) return;

	Niagara->Deactivate();

	Sprint(false);
	
	

}

void ANoviceCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void ANoviceCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}



void ANoviceCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (bElimmed) return;

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield-Damage,0.f,MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);

			Shield = 0.f;
			
		}

	}
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);//health is a replicated variable so it will update down to all clients when  this call on server
	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();//this function will only be called on clients as we bind this on server only
	
	if (Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if(BlasterGameMode)
		{
			NovicePlayerController = NovicePlayerController == nullptr ? Cast<ANovicePlayerController>(Controller) : NovicePlayerController;
			ANovicePlayerController* AttackerController = Cast<ANovicePlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this,NovicePlayerController,AttackerController);
		}
	}
	
}

void ANoviceCharacter::PollInit()
{
	if (NovicePlayerState==nullptr)
	{
		NovicePlayerState = GetPlayerState<ANovicePlayerState>();//PlayerState is not initialize at the begining(BeginPlay) of the game it is initiallize after passing some frame so as soon as it initialize we are going to call it in the tick 
		if (NovicePlayerState)
		{
			NovicePlayerState->AddToScore(0.f);
			NovicePlayerState->AddToDefeats(0);//updating without adding anything after elimination
		}
	
	}
}

