// Fill out your copyright notice in the Description page of Project Settings.


#include "NovicePlayerController.h"
#include "Blaster/HUD/NoviceHUD.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/NovicePlayerState.h"
#include "Blaster/Weapon/Weapon.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include"Blaster/HUD/ReturnToMainMenu.h"
#include "Blaster/Enums/AnnouncementTypes.h"

void ANovicePlayerController::BeginPlay()
{
	Super::BeginPlay();

	NoviceHUD = Cast<ANoviceHUD>(GetHUD());
	ServerCheckMatchState();
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}

 
}

void ANovicePlayerController::Tick(float DeltaTime)
{
	SetHUDTime();
	CheckTimeSync(DeltaTime);   
	PollInit();
	CheckPing(DeltaTime);
	
}

void ANovicePlayerController:: SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	//InputComponent->BindAction();
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent)) {

		EnhancedInputComponent->BindAction(ExitAction, ETriggerEvent::Triggered, this, &ANovicePlayerController::ShowReturnToMainMenu);

	}
}

void ANovicePlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		//we will get the ping from our player state
		if (PlayerState == nullptr)
		{
			PlayerState = GetPlayerState<APlayerState>();
		}
		else 
		{
			 PlayerState= PlayerState;
		}
			
		 
	 

		if (PlayerState)
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPing() * 4 : %f"), PlayerState->GetPingInMilliseconds() * 4);
			if (PlayerState->GetPingInMilliseconds() * 4 > HighPingThreshold) //Ping is Compressed it is actually divided by 4
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}

		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying = NoviceHUD &&
		NoviceHUD->CharacterOverlay &&
		NoviceHUD->CharacterOverlay->HighPingAnimation;
	if (bHighPingAnimationPlaying)
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}
 
void ANovicePlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}


void ANovicePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANovicePlayerController, MatchState);
	DOREPLIFETIME(ANovicePlayerController, bShowTeamScores);
}

void ANovicePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->HealthBar && NoviceHUD->CharacterOverlay->HealthText;
	if (bHUDIsValid)
	{
		const float HealthPercent = Health / MaxHealth;
		NoviceHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(FMath::Clamp(Health,0.f,MaxHealth)),FMath::CeilToInt(MaxHealth));
		NoviceHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ANovicePlayerController::SetHUDShield(float Shield, float MaxShield)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->ShieldBar && NoviceHUD->CharacterOverlay->ShieldText;
	if (bHUDIsValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		NoviceHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::FMath::CeilToInt(FMath::Clamp(Shield, 0.f, MaxShield)), FMath::CeilToInt(MaxShield));
		NoviceHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ANovicePlayerController::SetHUDScore(float Score)
{

	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->ScoreAmount;
	if (bHUDIsValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		NoviceHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeScore = true;
		HUDScore = Score;
	}
}

void ANovicePlayerController::SetHUDDefeats(int32 Defeats)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDIsValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		NoviceHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ANovicePlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDIsValid)
	{
		FString  AmmoText = FString::Printf(TEXT("%d"), Ammo);
		NoviceHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ANovicePlayerController::SetHUDCarriedWeaponAmmo(int32 Ammo)
{

	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDIsValid)
	{
		FString  AmmoText = FString::Printf(TEXT("%d"), Ammo);
		NoviceHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ANovicePlayerController::SetHUDCarriedGrenade(int32 Grenade)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->GrenadeAmount;
	if (bHUDIsValid)
	{
		FString  GrenadeText = FString::Printf(TEXT("%d"),Grenade);
		NoviceHUD->CharacterOverlay->GrenadeAmount->SetText(FText::FromString(GrenadeText));
	}
	else {
		bInitializeGrenades = true;
		HUDGrenades = Grenade;
	}
}

void ANovicePlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->MatchCountdownText ;
	if (bHUDIsValid)
	{
		if (CountdownTime < 0.f)
		{
			NoviceHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
 
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString  CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes,Seconds);
		NoviceHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ANovicePlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->Announcement && NoviceHUD->Announcement->WarmupTime;
	if (bHUDIsValid)
	{
		if (CountdownTime < 0.f)
		{
			NoviceHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString  CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
		NoviceHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

float ANovicePlayerController::GetServerTime()
{
	if (HasAuthority()) {
		return GetWorld()->GetTimeSeconds();
	}
	else {
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}
	
}
 
void ANovicePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(InPawn);
	if (NoviceCharacter)
	{
		SetHUDHealth(NoviceCharacter->GetHealth(), NoviceCharacter->GetMaxHealth());
	}
	if (NoviceCharacter)
	{
		SetHUDShield(NoviceCharacter->GetShield(), NoviceCharacter->GetMaxShield());
	}
 
	if (NoviceCharacter)
	{
		 SetHUDCarriedGrenade(NoviceCharacter->GetCombatComponent()->GetGrenades());
	}
 


	
}

void ANovicePlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;

	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; //because warmup time and levelStartingTime is already passed
	else if(MatchState==MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	//if we are on the server we should get countdown time directly from the server
/*
	if (HasAuthority())
	{
		BlasterGameMode = BlasterGameMode==nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);

		}
		}


*/

	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState==MatchState::Cooldown)
		{
			
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
		
	}
	
	CountdownInt = SecondsLeft;


}
  
void ANovicePlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (NoviceHUD && NoviceHUD->CharacterOverlay)
		{
			CharacterOverlay = NoviceHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if(bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedWeaponAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo)	SetHUDWeaponAmmo(HUDWeaponAmmo);
				
	
				
				ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(GetPawn());
				if (NoviceCharacter && NoviceCharacter->GetCombatComponent())
				{
					if (bInitializeGrenades)SetHUDCarriedGrenade(NoviceCharacter->GetCombatComponent()->GetGrenades());
				}
				
			}
		}
	}
}

void ANovicePlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());//if we are on the server it will pass down the server time either will pass the client's time
	}
}

void ANovicePlayerController::OnMatchStateSet(FName State,bool bTeamsMatch)
{

	//when it get called from the client Rpc this function will executed on the server and when the MatchState changes OnRep-MatchState Rep notify will be called on the clients
	//this only happens on server
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ANovicePlayerController::HideTeamScore()
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->RedTeamScoreText && NoviceHUD->CharacterOverlay->BlueTeamScoreText && NoviceHUD->CharacterOverlay->Spacer;
	if (bHUDIsValid)
	{
		 
		NoviceHUD->CharacterOverlay->RedTeamScoreText->SetText(FText());
		NoviceHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText());
		NoviceHUD->CharacterOverlay->Spacer->SetText(FText());
	}

}

void ANovicePlayerController::InitTeamScore()
{

	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->RedTeamScoreText && NoviceHUD->CharacterOverlay->BlueTeamScoreText && NoviceHUD->CharacterOverlay->Spacer;
	if (bHUDIsValid)
	{
		FString Zero("0");
		FString Spacer("|");

		NoviceHUD->CharacterOverlay->RedTeamScoreText->SetText(FText::FromString(Zero));
		NoviceHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText::FromString(Zero));
		NoviceHUD->CharacterOverlay->Spacer->SetText(FText::FromString(Spacer));
	}
}

void ANovicePlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->BlueTeamScoreText;
	if (bHUDIsValid)
	{
		FString  ScoreText = FString::Printf(TEXT("%d"),BlueScore);

 
		NoviceHUD->CharacterOverlay->BlueTeamScoreText->SetText(FText::FromString(ScoreText));
 
	}
}

void ANovicePlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->RedTeamScoreText;
	if (bHUDIsValid)
	{
		FString  ScoreText = FString::Printf(TEXT("%d"), RedScore);


		NoviceHUD->CharacterOverlay->RedTeamScoreText->SetText(FText::FromString(ScoreText));

	}
}

void ANovicePlayerController::OnRep_MatchState()
{
	
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ANovicePlayerController::HandleMatchHasStarted(bool bTeamsMatch )
{

		if(HasAuthority()) bShowTeamScores = bTeamsMatch;
		NoviceHUD = NoviceHUD == nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
		if (NoviceHUD)
		{
			if(NoviceHUD->CharacterOverlay==nullptr) NoviceHUD->AddCharacterOverlay();
			if (NoviceHUD->Announcement)
			{
				NoviceHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
			}
			if (!HasAuthority()) return;
			if (bTeamsMatch)
			{
				InitTeamScore();
			}
			else
			{
				HideTeamScore();
			}
		}
	
}

void ANovicePlayerController::HandleCooldown()
{
	NoviceHUD = NoviceHUD == nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	if (NoviceHUD)
	{
		NoviceHUD->CharacterOverlay->RemoveFromParent();

		bool bHUDValid = NoviceHUD->Announcement && 
			NoviceHUD->Announcement->announcementText && 
			NoviceHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			NoviceHUD->Announcement->SetVisibility(ESlateVisibility::Visible);

			FString AnnouncementText=  Announcement::NewMatchStartIn;
			NoviceHUD->Announcement->announcementText->SetText(FText::FromString(AnnouncementText));
			
			ABlasterGameState* BlasterGameState =Cast<ABlasterGameState>(UGameplayStatics:: GetGameState(this));
			ANovicePlayerState* NovicePlayerState = GetPlayerState<ANovicePlayerState>();
			if (BlasterGameState && NovicePlayerState)
			{
				TArray<ANovicePlayerState*> TopPlayers = BlasterGameState->TopScoringPlayer;
				FString InfoTextString= bShowTeamScores ? GetTeamInfoText(BlasterGameState) : GetInfoText(TopPlayers);
				 
				NoviceHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}

		
		
		}
	}
	ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(GetPawn());
	if (NoviceCharacter && NoviceCharacter->GetCombatComponent())
	{
		NoviceCharacter->bDisableGameplay = true;
		NoviceCharacter->GetCombatComponent()->FireButtonPressed(false);
	}

}

FString ANovicePlayerController::GetInfoText(const TArray<class ANovicePlayerState*>& Players)
{
	FString InfoTextString;
	ANovicePlayerState* NovicePlayerState = GetPlayerState<ANovicePlayerState>();
	if (NovicePlayerState == nullptr) return FString();
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == NovicePlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if ( Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner : %s"), * Players[0]->GetPlayerName()); //when only one player is in the top
	}
	else if ( Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayerTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer :  Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString ANovicePlayerController::GetTeamInfoText(ABlasterGameState* BlasterGameState)
{
	if(BlasterGameState==nullptr) return FString();

	FString InfoTextString;
	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), * Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWin;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::RedTeam,RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::BlueTeam,BlueTeamScore));
	}
	else if (RedTeamScore < BlueTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWin;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s:%d\n"), *Announcement::RedTeam, RedTeamScore));
		
	}
	return InfoTextString;
}
 
void ANovicePlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker,Victim);
}
   
void ANovicePlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
		if (NoviceHUD)
		{
			if (Attacker == Self && Victim!=Self)
			{
				NoviceHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				NoviceHUD->AddElimAnnouncement(Attacker->GetPlayerName(),"You");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				NoviceHUD->AddElimAnnouncement("You", "You");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				NoviceHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Themseleves");
				return;
			}
			NoviceHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());

		}
	}
}

void ANovicePlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		CooldownTime = GameMode->CooldownTime;
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime,CooldownTime);//passing down these value so they executed on the joining clients

		if (NoviceHUD && MatchState == MatchState::WaitingToStart)
		{
			if(NoviceHUD->Announcement==nullptr) NoviceHUD->AddAnnouncement();
		}
	
	}
}


void ANovicePlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch,float Warmup,float Match,float StartingTime,float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime=StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	//after this we can call OnMatchStateSet
	OnMatchStateSet(MatchState);

	if (NoviceHUD && MatchState == MatchState::WaitingToStart)
	{
		NoviceHUD->AddAnnouncement();
	}

}

void ANovicePlayerController::HighPingWarning()
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->HighPingImage && NoviceHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDIsValid)
	{
		NoviceHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		NoviceHUD->CharacterOverlay->PlayAnimation(NoviceHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

void ANovicePlayerController::StopHighPingWarning()
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->HighPingImage && NoviceHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDIsValid)
	{
		NoviceHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);

		if (NoviceHUD->CharacterOverlay->IsAnimationPlaying(NoviceHUD->CharacterOverlay->HighPingAnimation))
		{
			NoviceHUD->CharacterOverlay->StopAnimation(NoviceHUD->CharacterOverlay->HighPingAnimation);
		}
			
		}
}

void ANovicePlayerController::OnRep_bShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScore();
	}
	else
	{
		HideTeamScore();
	}
}


void ANovicePlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;

	}
}

void ANovicePlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu  == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this,ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
 
}

void ANovicePlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ANovicePlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime= 0.5f * RoundTripTime ;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

}


