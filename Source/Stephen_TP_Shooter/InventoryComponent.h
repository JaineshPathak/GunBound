// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class AItem;
class AWeapon;
class AAmmo;
class AShooterCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitchDelegate, int32, NewSlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAddedDelegate, int32, NewSlotIndex);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STEPHEN_TP_SHOOTER_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	TObjectPtr<AWeapon> SpawnDefaultWeapon();
	void DropWeapon(TObjectPtr<AWeapon> Weapon);
	void SwapWeapon(TObjectPtr<AWeapon> Weapon);
	void EquipWeapon(TObjectPtr<AWeapon> Weapon);
	void AttachWeaponToBody(TObjectPtr<AWeapon> Weapon);

private:
	void InitAmmoMap();

private:
	const int32 SLOT_PISTOL = 0;
	const int32 SLOT_SMG = 1;
	const int32 SLOT_AR = 2;
	const int32 SLOT_SHOTGUN = 3;
	const int32 INVENTORY_CAPACITY = 4;

	const FName SOCKET_SLOT_PISTOL = FName("WeaponPistolSlot");
	const FName SOCKET_SLOT_BACK_SMG = FName("WeaponBackSlotSMG");
	const FName SOCKET_SLOT_BACK_AR = FName("WeaponBackSlotAR");
	const FName SOCKET_SLOT_BACK_SHOTGUN = FName("WeaponBackSlotShotgun");

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (AllowPrivateAccess = "true"))
	FOnWeaponSwitchDelegate OnWeaponSwitchEvent;

	UPROPERTY(BlueprintAssignable, Category = "Events", meta = (AllowPrivateAccess = "true"))
	FOnWeaponAddedDelegate OnWeaponAddedEvent;

	//The Character Owner that owns this component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AShooterCharacter> Character;

	//Set this in BP for default weapon class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AWeapon> EquippedWeapon;

	//Currently equipped weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 InventoryCurrentSlot;

	//Array of Weapons
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<AWeapon>> Inventory;

	//Map to keep track of ammo of different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap;

	//Starting ammo of 9mm Ammo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	//Starting ammo of AR Ammo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	//Starting ammo of Shotgun Ammo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 StartingShotgunAmmo;

	//Starting ammo of Grenade
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 StartingGrenadeAmmo;

public:
	void SelectWeapon(TObjectPtr<AWeapon> Weapon);
	void SwitchToWeapon(int32 IndexSlot, bool& bShouldPlayEquipAnim);
	void AddWeapon(TObjectPtr<AWeapon> Weapon);
	
	bool WeaponHasAmmo();
	void DeductAmmo(EAmmoType AmmoType, int32 AmountToDeduct);
	void PickupAmmo(AAmmo* Ammo);
	bool HasAmmo();
	bool HasAmmoOfType(EAmmoType AmmoType);
	int32 GetSlotForWeapon(TObjectPtr<AWeapon> Weapon);
	void OnReloadFinish();

	UFUNCTION(BlueprintCallable)
	FString GetEquippedWeaponAmmo();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetGrenadeAmmoCount() const { return AmmoMap[EAmmoType::EAT_Grenade]; }
	
	FORCEINLINE TObjectPtr<AWeapon> GetEquippedWeapon() const { return EquippedWeapon; }
};