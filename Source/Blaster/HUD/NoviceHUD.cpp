// Fill out your copyright notice in the Description page of Project Settings.


#include "NoviceHUD.h"
#include "CharacterOverlay.h"
#include "Announcement.h"

void ANoviceHUD::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCentre(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		
		if (HUDPackage.CrosshairsCentre) {
			FVector2D Spread(0.f, 0.f);
			DrawCrosshairs(HUDPackage.CrosshairsCentre, ViewportCentre,Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsLeft) {
			FVector2D Spread(-SpreadScaled, 0.f);//from this amount the left crosshairs will spread in left direction by SpreadScaled times dynamically
			DrawCrosshairs(HUDPackage.CrosshairsLeft, ViewportCentre,Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsRight) {
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshairs(HUDPackage.CrosshairsRight, ViewportCentre,Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsBottom) {
			FVector2D Spread(0.f,SpreadScaled);//pushing the crossjairs in positive Y direction
			DrawCrosshairs(HUDPackage.CrosshairsBottom, ViewportCentre, Spread, HUDPackage.CrosshairsColor);
		}
		if (HUDPackage.CrosshairsTop) {
			FVector2D Spread(0.f,-SpreadScaled);
			DrawCrosshairs(HUDPackage.CrosshairsTop, ViewportCentre,Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ANoviceHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ANoviceHUD::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANoviceHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController=GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController,CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ANoviceHUD::DrawCrosshairs(UTexture2D* Texture, FVector2D ViewportCentre, FVector2D Spread, FLinearColor CrosshairsColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(
	ViewportCentre.X -(TextureWidth/2.f)+Spread.X,
	ViewportCentre.Y-(TextureHeight/2.f)+Spread.Y
	);
	DrawTexture(Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairsColor);
}
