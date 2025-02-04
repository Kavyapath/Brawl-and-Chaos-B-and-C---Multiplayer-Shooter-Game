// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include"Blaster/Character/NoviceCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/PlayerController/NovicePlayerController.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"



AWeapon::AWeapon()
{
 
	PrimaryActorTick.bCanEverTick = false;
	//our weapon is replicated actor so we have to pass replication true so teh server will be the incharge of all the weapon instances
	bReplicates = true;
	SetReplicateMovement(true);
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	//We want that area sphere detect overlap events when we are on the server
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
	






	WeaponMesh->SetCustomDepthStencilValue(CUSTUM_DEPTH_BLUE);//for weapon highlight
	WeaponMesh->MarkRenderStateDirty();
	EnabledCustumDepth(true);//all weapon will start off with custom depth enabled
}


void AWeapon::BeginPlay()
{
	Super::BeginPlay();


	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);


	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
 
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(OtherActor);
	if (NoviceCharacter)
	{
		if (WeaponType == EWeaponTypes::EWT_Flag && NoviceCharacter->GetTeam() == Team) return;
		if (NoviceCharacter->IsHoldingTheFlag()) return;
		NoviceCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(OtherActor);
	if (NoviceCharacter)
	{
		if (WeaponType == EWeaponTypes::EWT_Flag && NoviceCharacter->GetTeam() == Team) return;
		if (NoviceCharacter->IsHoldingTheFlag()) return;
		NoviceCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	
	if (HasAuthority())
	{
		ClientAddAmmo(AmmoToAdd);
	}

}


void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{

	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;
	if (NoviceOwnerCharacter && NoviceOwnerCharacter->GetCombatComponent() && IsFull())
	{
		NoviceOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();

}

void AWeapon::SpendRound()
{
	Ammo=FMath::Clamp(Ammo - 1 ,0,MagCapacity);
	SetHUDAmmo();

	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);//server will send the client RPC on all the client machines
	}
	else
	{
		++Sequence;
	}


}

//client side Prediction or server Reconciliation algorithm
void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	//Storing all the unprocessed request (RPC) in a sequence
	Ammo = ServerAmmo;
	//Request answer has come in the form of client RPC from the server
	--Sequence;
	Ammo -= Sequence;//we know that we have spent that many rounds
	SetHUDAmmo();
}


/* using Client side Prediction for ammo
void AWeapon::OnRep_Ammo()
{
	NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;

	if (NoviceOwnerCharacter && NoviceOwnerCharacter->GetCombatComponent() && IsFull())
	{
		NoviceOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}

*/

void AWeapon::OnRep_Owner()
{

	Super::OnRep_Owner();//as soon as owner is replicated the ammo will be set on all the  client machine 
	if (Owner == nullptr)
	{
		NoviceOwnerCharacter = nullptr;
		NoviceOwnerController = nullptr;
	}
	else
	{
		NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(Owner) : NoviceOwnerCharacter;
		if (NoviceOwnerCharacter && NoviceOwnerCharacter->GetEquippedWeapon() && NoviceOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
		
	}
	
}

void AWeapon::SetHUDAmmo()
{
	NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;
	if (NoviceOwnerCharacter)
	{
		NoviceOwnerController = NoviceOwnerController == nullptr ? Cast<ANovicePlayerController>(NoviceOwnerCharacter->Controller) : NoviceOwnerController;
		if (NoviceOwnerController)
		{
			NoviceOwnerController->SetHUDWeaponAmmo(Ammo);
		}

	}
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalize = (HitTarget - TraceStart).GetSafeNormal();

	const FVector SphereCenter = TraceStart + ToTargetNormalize * DistanceToSphere;

	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);

	const FVector EndLoc = SphereCenter + RandVec;

	const FVector ToEndLoc = EndLoc - TraceStart;
	/*
		DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red,true);
		DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Blue, true);

		DrawDebugLine(GetWorld(), TraceStart,
			FVector(TraceStart + ToEndLoc * Trace_Lenght / ToEndLoc.Size()),
		FColor::Cyan,
			true);

	*/
	return FVector(TraceStart + ToEndLoc * Trace_Lenght / ToEndLoc.Size());
}

void AWeapon::EnabledCustumDepth(bool bEnabled)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnabled);
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;


	}
}

void AWeapon::OnDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);


	WeaponMesh->SetCustomDepthStencilValue(CUSTUM_DEPTH_BLUE);//for weapon highlight
	WeaponMesh->MarkRenderStateDirty();
	EnabledCustumDepth(true);

	NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;
	if (NoviceOwnerCharacter && bUseServerSideRewind)
	{
		NoviceOwnerController = NoviceOwnerController == nullptr ? Cast<ANovicePlayerController>(NoviceOwnerCharacter->Controller) : NoviceOwnerController;
		if (NoviceOwnerController && HasAuthority() && !NoviceOwnerController->HighPingDelegate.IsBound())
		{
			NoviceOwnerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}

	}
}

void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	//we are generating the overlap events for area sphere on the server so after equiping the weapon we will disable its collision on the server so the areaSphere does not generate overlap events with other clients when it is picked by one client
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponTypes::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//for enabling stips physics
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	EnabledCustumDepth(false);

	NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;
	if (NoviceOwnerCharacter && bUseServerSideRewind)
	{
		NoviceOwnerController = NoviceOwnerController == nullptr ? Cast<ANovicePlayerController>(NoviceOwnerCharacter->Controller) : NoviceOwnerController;
		if (NoviceOwnerController && HasAuthority() && !NoviceOwnerController->HighPingDelegate.IsBound())
		{
			NoviceOwnerController->HighPingDelegate.AddDynamic(this,&ThisClass::OnPingTooHigh);
		}

	}
	
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	//we are generating the overlap events for area sphere on the server so after equiping the weapon we will disable its collision on the server so the areaSphere does not generate overlap events with other clients when it is picked by one client
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponTypes::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//for enabling stips physics
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}

	 
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTUM_DEPTH_TAN);//for weapon highlight
		WeaponMesh->MarkRenderStateDirty();

	}

	NoviceOwnerCharacter = NoviceOwnerCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceOwnerCharacter;
	if (NoviceOwnerCharacter && bUseServerSideRewind)
	{
		NoviceOwnerController = NoviceOwnerController == nullptr ? Cast<ANovicePlayerController>(NoviceOwnerCharacter->Controller) : NoviceOwnerController;
		if (NoviceOwnerController && HasAuthority() && !NoviceOwnerController->HighPingDelegate.IsBound())
		{
			NoviceOwnerController->HighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}

	}
}
 


void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();


}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


bool AWeapon::IsEmpty()
{
	return Ammo <=0;
}

bool AWeapon::IsFull()
{
	return Ammo==MagCapacity;
}




void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
	//DOREPLIFETIME(AWeapon, Ammo);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind,COND_OwnerOnly);

}



void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation,false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEject = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEject)
		{
			FTransform SocketTransform = AmmoEject->GetSocketTransform(WeaponMesh);
		
		
			{
			

				UWorld* World = GetWorld();
				if (World) {
					World->SpawnActor<ACasing>(CasingClass,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()
						
					);
				}

			}

		}
	}
	 SpendRound();
	
	

}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld,true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	SetInstigator(nullptr);
	//when we are dropping the weapon it should not have the pointers set for the previous owner 
	NoviceOwnerCharacter = nullptr;
	NoviceOwnerController = nullptr;
}

