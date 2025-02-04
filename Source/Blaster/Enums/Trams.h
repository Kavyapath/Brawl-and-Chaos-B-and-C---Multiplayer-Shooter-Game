#pragma once

UENUM(BlueprintType)
enum class ETeams : uint8
{
	ET_RedTeam UMETA(DisplayName = "RedTeam"),
	ET_BlueTeam UMETA(DisplayName = "BlueTeam"),
	ET_NoTeam UMETA(DisplayName = "NoTeam"),

	ET_Max UMETA(DisplayName = "DefaultMax") 

};
