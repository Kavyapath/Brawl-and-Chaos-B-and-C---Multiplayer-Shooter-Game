// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NovicePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ANovicePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedWeaponAmmo(int32 Ammo);
	void SetHUDCarriedGrenade(int32 Grenade);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual float GetServerTime();//synced with server world clock
	virtual void ReceivedPlayer() override;//Sync with server clock as soon as possible,as soon as client attach with controller it will automatically evoked
	void OnMatchStateSet(FName State);

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;///called automatically when player is possess by the controller
	void SetHUDTime();
	void PollInit();
	/*
	sync time between client and server
	*/

	//Request the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	//Report the current server time to ythe client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest,float TimeServerReceivedClientRequest);//called on the server and executed on the all client machine
	
	float ClientServerDelta = 0.f;// difference between client and server time
	
	UPROPERTY(EditAnywhere,Category=Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime,float Cooldown);
private:

	void CheckTimeSync(float DeltaTime);

	UPROPERTY()
	class ANoviceHUD* NoviceHUD;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float LevelStartingTime = 0.f;
	float CooldownTime = 0.f;

	uint32 CountdownInt=0;

	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	void HandleMatchHasStarted();
	void HandleCooldown();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;/// to initialize the character overlay after 10 seconds of waiting time

	bool bInitializeCharacterOverlay=false;
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	int32 HUDGrenades;
};
