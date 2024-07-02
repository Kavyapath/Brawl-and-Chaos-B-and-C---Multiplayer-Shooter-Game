// Fill out your copyright notice in the Description page of Project Settings.


#include "NovicePlayerController.h"
#include "Blaster/HUD/NoviceHUD.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/NovicePlayerState.h"

void ANovicePlayerController::BeginPlay()
{
	Super::BeginPlay();

	NoviceHUD = Cast<ANoviceHUD>(GetHUD());
	ServerCheckMatchState();




}

void ANovicePlayerController::Tick(float DeltaTime)
{
	SetHUDTime();
	CheckTimeSync(DeltaTime);   
	PollInit();
	
}

void ANovicePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANovicePlayerController, MatchState);
}

void ANovicePlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	NoviceHUD = NoviceHUD = nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
	bool bHUDIsValid = NoviceHUD && NoviceHUD->CharacterOverlay && NoviceHUD->CharacterOverlay->HealthBar && NoviceHUD->CharacterOverlay->HealthText;
	if (bHUDIsValid)
	{
		const float HealthPercent = Health / MaxHealth;
		NoviceHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health),FMath::CeilToInt(MaxHealth));
		NoviceHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
		bInitializeCharacterOverlay = true;
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
		bInitializeCharacterOverlay = true;
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
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
				ANoviceCharacter* NoviceCharacter = Cast<ANoviceCharacter>(GetPawn());
				if (NoviceCharacter && NoviceCharacter->GetCombatComponent())
				{
					SetHUDCarriedGrenade(NoviceCharacter->GetCombatComponent()->GetGrenades());
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

void ANovicePlayerController::OnMatchStateSet(FName State)
{

	//when it get called from the client Rpc this function will executed on the server and when the MatchState changes OnRep-MatchState Rep notify will be called on the clients
	//this only happens on server
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
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

void ANovicePlayerController::HandleMatchHasStarted()
{

		NoviceHUD = NoviceHUD == nullptr ? Cast<ANoviceHUD>(GetHUD()) : NoviceHUD;
		if (NoviceHUD)
		{
			if(NoviceHUD->CharacterOverlay==nullptr) NoviceHUD->AddCharacterOverlay();
			if (NoviceHUD->Announcement)
			{
				NoviceHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
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

			FString AnnouncementText("New Match Start In:");
			NoviceHUD->Announcement->announcementText->SetText(FText::FromString(AnnouncementText));
			
			ABlasterGameState* BlasterGameState =Cast<ABlasterGameState>(UGameplayStatics:: GetGameState(this));
			ANovicePlayerState* NovicePlayerState = GetPlayerState<ANovicePlayerState>();
			if (BlasterGameState && NovicePlayerState)
			{
				TArray<ANovicePlayerState*> TopPlayers = BlasterGameState->TopScoringPlayer;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == NovicePlayerState)
				{
					InfoTextString = FString("You are the Winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner : %s"), *TopPlayers[0]->GetPlayerName()); //when only one player is in the top
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for win: \n");
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
			
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

void ANovicePlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;

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
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();

}


