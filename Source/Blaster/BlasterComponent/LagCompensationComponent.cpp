// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Blaster/Character/NoviceCharacter.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"


ULagCompensationComponent::ULagCompensationComponent()
{

	PrimaryComponentTick.bCanEverTick = true;

	  
}



void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();



	
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	NoviceCharacter = NoviceCharacter == nullptr ? Cast<ANoviceCharacter>(GetOwner()) : NoviceCharacter;
	if (NoviceCharacter)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : NoviceCharacter->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);

		}
	}

}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time ) / Distance,0,1);

	FFramePackage InterpFramePackage;

	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;


		const FBoxInformation& OlderBoxInfo = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		//Interpolate between these two FBoxInformations and each time we gonna made the new FBoxInformation

		FBoxInformation InterpBoxInfo;

		InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBox.Location, 1, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBox.Rotation, 1, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);

	}
	return InterpFramePackage;
}



void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);//0,1 elements are covered in this

	}
	else 
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while(HistoryLength > MaxRecordTime)//time to discard element from the linklist
		{
		
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;

		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		ShowFramePackage(ThisFrame,FColor::Red);

	}


}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
		GetWorld(),
		BoxInfo.Value.Location,
		BoxInfo.Value.BoxExtent,
		FQuat(BoxInfo.Value.Rotation),
		Color,
		false,
		4.f);

	}
}

void ULagCompensationComponent::ServerSideRewind(ANoviceCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	
	if (bReturn) return;

	//Frame Package that we check to varify a hit
	FFramePackage FrameToCheck;
	bool bShouldInterpolate =true;

	//Frame History of the HitCharacter 
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (OldHistoryTime > HitTime)
	{
		//hit time was before the oldHistoryTime
		//too far back - too laggy to do SSR
		return;
	}
	if (OldHistoryTime == HitTime)
	{
		FrameToCheck= History.GetTail()->GetValue();
	}
	if (NewestHistoryTime <=HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;

	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime)// is older still younger than hitTime ? 
	{
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();

		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	if (Older->GetValue().Time == HitTime)//highly unlikely, but we found our frame to check
	{
		FrameToCheck = Older->GetValue(); 
		bShouldInterpolate = false;//because we find the exact spot of HitTime
	}

	if (bShouldInterpolate)
	{
		//Interpolate Between younger and older
	}


}

