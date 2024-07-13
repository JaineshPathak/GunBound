// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AmmoType.h"
#include "ShooterCharacter.generated.h"

class AGrenade;
class AWeapon;
class AItem;
class AAmmo;
class AShooterPlayerController;
class UHealthComponent;
class UInventoryComponent;
class USoundsDataAsset;

class UAudioComponent;
class USpringArmComponent;
class UCameraComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied				UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress		UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading				UMETA(DisplayName = "Reloading"),
	ECS_Equipping				UMETA(DisplayName = "Equipping"),
	ECS_PickingUp				UMETA(DisplayName = "Picking Up"),
	ECS_Throwing				UMETA(DisplayName = "Throwing"),
	ECS_Stunned					UMETA(DisplayName = "Stunned"),

	ECS_MAX						UMETA(DisplayName = "Default MAX")
};

USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	//Used to get location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	//Number of items interping to/at SceneComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;

	FInterpLocation() :
		SceneComponent(nullptr),
		ItemCount(0)
	{}

	FInterpLocation(USceneComponent* SceneComp, const int32& ItemNum) :
		SceneComponent(SceneComp),
		ItemCount(ItemNum)
	{}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemsDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShooterDeathDelegate);

UCLASS()
class STEPHEN_TP_SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

	friend class AWeapon;

public:
	AShooterCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void FinishThrowingGrenade();

	UFUNCTION(BlueprintCallable)
	void FinishPickup();

	//Called from Animation BP with GrabClip
	UFUNCTION(BlueprintCallable)
	void GrabClip();

	//Called from Animation BP with ReleaseClip
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetFootstepSurfaceType();

	UFUNCTION(BlueprintCallable)
	void EndStun();

	UFUNCTION()
	void OnDeath(AActor* InstigatorActor, AController* InstigatorController);

	void CameraInterpAim(float DeltaTime);
	void CalcCrosshairSpread(float DeltaTime);
	void InterpCapsuleHeights(float DeltaTime);

	void StartCrosshairBulletFire();

	//========================================================================================

	// Line Trace for Under Crosshair
	bool TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation);

	//Trace for Items if Overlapped Items is greater than 0
	void TraceForItems();

	//Fire Weapon Functions
	void PlayGunFireMontage();

	//Single function for playing a montage provided with section name
	void PlayAnAnimMontage(TObjectPtr<UAnimMontage> MontageToPlay, FName MontageSectionName = NAME_None);

	void Aim();
	void StopAiming();

	void InitInterpLocations();	

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	void Stun();
	void DetermineStunChance();

	//Time to wait before we play another pickup sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;

	//Time to wait before we play another equip sound
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

public:	
	FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);	

	int32 GetInterpLocationIndex();

	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	void AimingPressed();
	void AimingReleased();
	void CrouchPressed();

	void FiringPressed();
	void FiringReleased();
	
	void FireWeapon();
	void ReloadWeapon();
	void ThrowGrenade();
	void SwitchToWeapon(int32 NewItemIndex);
	void Interact();
	void PlayKillTauntSound();
	
private:
	//Health and Armor Stats for Player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	//Audio Components that plays various sounds from SoundsDataAsset
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AudioComponent;

	//Stores all sounds data for each character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundsDataAsset> SoundsDataAsset;

	//Inventory class that takes care of Weapons and Ammo for the player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	//Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HipFireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> PickupMontage;

	//True when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	//Default Camera Field of View
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraDefaultFOV;

	//Aiming Camera Field of View
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraAimingFOV;

	//Current Field of View
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float CameraCurrentFOV;

	//Threshold FOV for allowing the player to start shooting when holding the primary fire button
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float AimShootFOVThreshold;

	//Interp Speed for Aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AimingInterpSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanFireWeapon;

	//Determines the spread of crosshair
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	//Velocity Component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	//In-Air Component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	//Aim Component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	//Shooting Component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	//True, if trace every frame for items
	bool bShouldTraceForItems;

	//Number of overlapped items
	int8 OverlappedItemCount;

	//The AItem we hit last Item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	AItem* TracedItemLastFrame;

	//Set this in BP for default grenade class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AGrenade> DefaultGrenadeClass;

	//The item current hit trace in TraceForItems()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	//Distance from Camera Interp Location forward
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	//Distance from Camera Interp Location Up
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	//Combat State when reloading or unoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	//Montage for Reload Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ReloadMontage;

	//Montage for Reload Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> EquipMontage;

	//Transform of the Clip Mag Transform in Skeleton tree
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	//Scene Component to attach to the character's hand
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	//True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	//Normal movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	//Crouch movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	//Current Half Height of Capsule every frame
	float CurrentCapsuleHalfHeight;

	//Half height of capsule when standing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	//Half height of capsule when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	//Ground friction while not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	//Ground friction while crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;

	//Array of Interp Location Structs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	//Delegate for sending slot information to inventory bar when equipping
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FEquipItemsDelegate EquipItemDelegate;

	//Delegate for sending slot information for playing Icon Highlight Animation
	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FOnShooterDeathDelegate OnShooterDeath;

	//Index for Currently Highlighted Icon Slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;

	//Blood Particles when taking damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> BloodParticles;

	//Hit React Anim Montage for when Character is Stunned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HitReactMontage;

	//Stun Chance when hit by an enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	//Throw Grenade Montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> ThrowGrenadeMontage;

	//Play Sound when throwing grenade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ThrowGrenadeTauntTime;

	//Play Sound when throwing grenade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float KillTauntTime;

	bool bTauntInGrenadeThrow;
	FTimerHandle ThrowGrenadeTauntTimer;

	bool bCanKillTaunt;
	FTimerHandle KillTauntTimer;

	TObjectPtr<AShooterPlayerController> ShooterPlayerController;
 
public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCrosshairSpreadMultiplier() const { return CrosshairSpreadMultiplier; }

	UFUNCTION(BlueprintCallable)
	void PlayTheSound(const FString& SoundName, bool bFadeIn, bool bFadeOut, float VolumeMultiplier);

	FORCEINLINE TObjectPtr<USpringArmComponent> GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE TObjectPtr<UCameraComponent> GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE TObjectPtr<UInventoryComponent> GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE TObjectPtr<UHealthComponent> GetHealthComponent() const { return HealthComponent; }
	FORCEINLINE TObjectPtr<UParticleSystem> GetBloodParticles() const { return BloodParticles; }
	FORCEINLINE bool GetAimingStatus() const { return bAiming; }
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }
	FORCEINLINE ECombatState GetCombateState() const { return CombatState; }
	FORCEINLINE bool GetCrouchingStatus() const { return bCrouching; }
	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }
	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }
	FORCEINLINE float GetStunChance() const { return StunChance; }

	void IncrementOverlappedItemCount(int8 Amount);
	TObjectPtr<AShooterPlayerController> GetShooterController();

	FInterpLocation GetInterpLocation(int32 Index);

	void StartPickupSoundTimer();
	void StartEquipSoundTimer();
};
