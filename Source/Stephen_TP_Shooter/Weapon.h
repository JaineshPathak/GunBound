// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "WeaponType.h"

#include "Engine/DataTable.h"
#include "Weapon.generated.h"

class USoundCue;
class UParticleSystem;
class UParticleSystemComponent;

UENUM(BlueprintType)
enum class EFiringMode : uint8
{
	EFM_SemiAuto		UMETA(DisplayName = "Semi Automatic"),
	EFM_Auto			UMETA(DisplayName = "Automatic"),
	EFM_Shotgun			UMETA(DisplayName = "Shotgun"),

	EFM_MAX				UMETA(DisplayName = "Default MAX")
};

USTRUCT(BlueprintType)
struct FWeaponRarityDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageHeadshot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Accuracy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecoilPitch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RecoilYaw;

	FWeaponRarityDataTable() :
		DamageBody(10.0f),
		DamageHeadshot(15.0f),
		DamageMultiplier(1.1f),
		Accuracy(5.0f),
		RecoilPitch(1.0f),
		RecoilYaw(1.0f)
	{}
};

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFiringMode FireMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundCue> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundCue> EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMesh> ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> AmmoIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> CrosshairBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CrosshairDefaultSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundCue> FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;

	FWeaponDataTable() :
		FireMode(EFiringMode::EFM_SemiAuto),
		AmmoType(EAmmoType::EAT_9mm),
		WeaponAmmo(30),
		MagCapacity(30),
		PickupSound(nullptr),
		EquipSound(nullptr),
		ItemMesh(nullptr),
		ItemName(FString("Weapon")),
		InventoryIcon(nullptr),
		AmmoIcon(nullptr),
		MaterialInstance(nullptr),
		MaterialIndex(0),
		ClipBoneName(NAME_None),
		ReloadMontageSection(NAME_None),
		AnimBP(UAnimInstance::StaticClass()),
		CrosshairMiddle(nullptr),
		CrosshairLeft(nullptr),
		CrosshairRight(nullptr),
		CrosshairTop(nullptr),
		CrosshairBottom(nullptr),
		CrosshairDefaultSpread(0.5f),
		AutoFireRate(0.1f),
		MuzzleFlash(nullptr),
		FireSound(nullptr),
		BoneToHide(NAME_None)
	{}
};

UCLASS()
class STEPHEN_TP_SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
	void OnFireTimerFinish();

	UFUNCTION()
	void StopFalling();

	void UpdateSlideDisplacement();

	bool TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation);
	bool GetTraceEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);

	//Type of Firing Mode of the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EFiringMode FireMode;

	//Ammo Count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	//Max Ammo of weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagCapacity;

	//Type of Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	//Type of Ammo for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	//FName for reload montage section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	//True when moving the clip while reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	//Bone name of the magaxine of the weapon so that the character grabs it when playing reloading animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName;

	//Index of Previous Dynamic Material so that it can be cleared when WeaponType is changed and a new Dynamic Material Index is created
	int32 PreviousMaterialIndex;

	//Centered Crosshair Texture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairMiddle;

	//Left side Crosshair Texture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairLeft;

	//Right side Crosshair Texture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairRight;

	//Top side Crosshair Texture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairTop;

	//Bottom side Crosshair Texture
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> CrosshairBottom;

	//Firing Rate of Weapon when firing mode is set to EFiringMode::EFM_Auto
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;

	//Play Firing Particle Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> MuzzleFlashEffect;

	//Firing Particle Effects Component
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystemComponent> MuzzleFlashComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> BeamParticleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> BulletWallHitEffect;

	//Plays Fire Sound when firing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> FireSound;

	//Name of bone to hide part of weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;

	//Amount that the slide is pushed during pistol fire
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float SlideDisplacement;

	//Curve for the Slide Displacement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> SlideDisplacementCurve;

	//Time for displacing the slide 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float SlideDisplacementTime;

	//True when moving pistol slide
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide;

	//Max displacement for slide on pistol
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;

	//Max rotation for Pistol Recoil
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float MaxRecoilRotation;

	//Amount that the pistol will rotate during pistol fire
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Pistol Properties", meta = (AllowPrivateAccess = "true"))
	float RecoilRotation;

	//Normal Body damage when hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Damage;

	//Normal Body damage when hit something
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float DamageMultiplier;

	//Headshot Damage when hit in head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float HeadshotDamage;

	//Headshot Damage when hit in head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float CrosshairDefaultSpread;

	//Accuracy of Weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Accuracy;

	//Up/Down Axis Recoil when firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float RecoilPitch;

	//Left/Right Axis Recoil when firing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float RecoilYaw;

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	FTimerHandle FireTimerHandle;

	//Timer Handle for Updating Pistol Slide Displacement
	FTimerHandle SlideTimer;

	const USkeletalMeshSocket* BarrelSocket;

public:
	void ThrowWeapon();
	void Fire();
	void FireBullet();
	void ReloadAmmo(int32 Amount);
	void StartSlideTimer();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetAmmoPercent() const { return (float)Ammo / (float)MagCapacity; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetAmmoClipText() const { return FString::FromInt(Ammo) + "/" + FString::FromInt(MagCapacity); }
	
	FORCEINLINE bool ClipIsFull() const { return Ammo >= MagCapacity; }

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE void SetReloadMontageSection(const FName& NewMontageSection) { ReloadMontageSection = NewMontageSection; }

	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetClipBoneName(const FName& NewBoneName) { ClipBoneName = NewBoneName; }

	FORCEINLINE bool GetMovingClip() const { return bMovingClip; }
	FORCEINLINE void SetMovingClip(const bool& move) { bMovingClip = move; }

	FORCEINLINE float GetCrosshairDefaultSpread() const { return CrosshairDefaultSpread; }
	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	
	FORCEINLINE TObjectPtr<USoundCue> GetFireSound() const { return FireSound; }
	FORCEINLINE const USkeletalMeshSocket* GetBarrelSocket() const { return BarrelSocket; }

	FORCEINLINE float GetDamage() const { return Damage * DamageMultiplier; }
	FORCEINLINE float GetHeadshotDamage() const { return HeadshotDamage * DamageMultiplier; }
	FORCEINLINE float GetDamageMultiplier() const { return DamageMultiplier; }
	FORCEINLINE float GetAccuracy() const { return Accuracy; }
};
