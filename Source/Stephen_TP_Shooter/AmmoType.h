#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_AR UMETA(DisplayName = "Assault Rifle"),
	EAT_Grenade UMETA(DisplayName = "Grenade"),
	EAT_Shotgun UMETA(DisplayName = "Shotgun"),

	EAT_MAX UMETA(DisplayName = "Default MAX")
};