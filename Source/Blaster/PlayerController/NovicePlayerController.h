// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "NovicePlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);
/**
 * 
 */
UCLASS()
class BLASTER_API ANovicePlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;

	void CheckPing(float DeltaTime);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const  override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);

	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedWeaponAmmo(int32 Ammo);
	void SetHUDCarriedGrenade(int32 Grenade);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual float GetServerTime();//synced with server world clock
	virtual void ReceivedPlayer() override;//Sync with server clock as soon as possible,as soon as client attach with controller it will automatically evoked
	void OnMatchStateSet(FName State,bool bTeamsMatch=false);
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	void HideTeamScore();
	void InitTeamScore();

	void SetHUDBlueTeamScore(int32 BlueScore);
	void SetHUDRedTeamScore(int32 RedScore);

	float SingleTripTime=0.f;

	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);// we will call it from the game mode (Server)


private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ExitAction;

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;///called automatically when player is possess by the controller
	void SetHUDTime();
	void PollInit();
	virtual void SetupInputComponent() override;
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

	void HighPingWarning();
	void StopHighPingWarning();
	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing=OnRep_bShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_bShowTeamScores();

	FString GetInfoText(const TArray<class ANovicePlayerState*>& Players);
	FString GetTeamInfoText(class ABlasterGameState* BlasterGameState);
private:

	void CheckTimeSync(float DeltaTime);


	/*
	Return To MainMenu
	*/
	UPROPERTY(EditAnywhere,Category=Widget)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;



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


	void HandleCooldown();

	
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;/// to initialize the character overlay after 10 seconds of waiting time

	
	float HUDHealth;
	bool bInitializeHealth = false;

	float HUDMaxHealth;
	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDShield;
	float HUDMaxShield;

	bool bInitializeShield = false;

	bool bInitializeWeaponAmmo = false;
	float HUDWeaponAmmo;

	bool bInitializeCarriedAmmo = false;
	float HUDCarriedAmmo;


	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server,Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	UPROPERTY(EditAnywhere)
	float PingAnimationRunningTime = 0.f;
};
