// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Item.h"
#include "Weapon.h"
#include "Ammo.h"
#include "ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"

UInventoryComponent::UInventoryComponent() : 
	InventoryCurrentSlot(0),
	Starting9mmAmmo(95),
	StartingARAmmo(120),
	StartingShotgunAmmo(40),
	StartingGrenadeAmmo(5)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AShooterCharacter>(GetOwner());

	for (int i = 0; i < INVENTORY_CAPACITY; i++)
		Inventory.Add(nullptr);

	InitAmmoMap();
	EquipWeapon(SpawnDefaultWeapon());
}

TObjectPtr<AWeapon> UInventoryComponent::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass == nullptr) return nullptr;

	TObjectPtr<AWeapon> Weapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	Weapon->DisableCustomDepth();
	Weapon->DisableGlowMaterial();
	Weapon->SetCharacter(Character);

	AddWeapon(Weapon);

	Weapon->SetItemState(EItemState::EIS_Equipped);

	return Weapon;
}

void UInventoryComponent::DropWeapon(TObjectPtr<AWeapon> Weapon)
{
	if (Weapon == nullptr) return;

	FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
	Weapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
	Weapon->SetItemState(EItemState::EIS_Falling);
	Weapon->ThrowWeapon();
}

void UInventoryComponent::SwapWeapon(TObjectPtr<AWeapon> Weapon)
{
	DropWeapon(EquippedWeapon);
	EquipWeapon(Weapon);
}

void UInventoryComponent::EquipWeapon(TObjectPtr<AWeapon> Weapon)
{
	if (Weapon == nullptr || Character == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
		HandSocket->AttachActor(Weapon, Character->GetMesh());

	EquippedWeapon = Weapon;
	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);

	InventoryCurrentSlot = GetSlotForWeapon(EquippedWeapon);

	OnWeaponSwitchEvent.Broadcast(InventoryCurrentSlot);
}

void UInventoryComponent::AttachWeaponToBody(TObjectPtr<AWeapon> Weapon)
{
	if (Weapon == nullptr || Character == nullptr) return;
	
	FName SocketName = NAME_None;

	switch (GetSlotForWeapon(Weapon))
	{
	case 0:	SocketName = SOCKET_SLOT_PISTOL;	break;
	case 1:	SocketName = SOCKET_SLOT_BACK_SMG;	break;
	case 2:	SocketName = SOCKET_SLOT_BACK_AR;	break;
	case 3:	SocketName = SOCKET_SLOT_BACK_SHOTGUN;	break;
	}

	const USkeletalMeshSocket* AttachSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (AttachSocket)
		AttachSocket->AttachActor(Weapon, Character->GetMesh());
}

int32 UInventoryComponent::GetSlotForWeapon(TObjectPtr<AWeapon> Weapon)
{
	if (Weapon == nullptr) return -1;

	int32 WeaponSlot = -1;

	switch (Weapon->GetWeaponType())
	{
	case EWeaponType::EWT_Pistol:			WeaponSlot = SLOT_PISTOL;	break;
	case EWeaponType::EWT_SubmachineGun:	WeaponSlot = SLOT_SMG;		break;
	case EWeaponType::EWT_AssaultRifle:		WeaponSlot = SLOT_AR;		break;
	case EWeaponType::EWT_Shotgun:			WeaponSlot = SLOT_SHOTGUN;	break;
	}

	return WeaponSlot;
}

void UInventoryComponent::InitAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
	AmmoMap.Add(EAmmoType::EAT_Grenade, StartingGrenadeAmmo);
	AmmoMap.Add(EAmmoType::EAT_Shotgun, StartingShotgunAmmo);
}

void UInventoryComponent::SelectWeapon(TObjectPtr<AWeapon> Weapon)
{
	if(Weapon == nullptr || Character == nullptr) return;

	//Weapon->StartItemCurve(Character, true);
	Weapon->FinishInterping();
}

void UInventoryComponent::SwitchToWeapon(int32 IndexSlot, bool& bShouldPlayEquipAnim)
{
	if (Inventory[IndexSlot] == nullptr || GetSlotForWeapon(EquippedWeapon) == IndexSlot) return;

	bShouldPlayEquipAnim = true;

	auto OldEquippedWeapon = EquippedWeapon;
	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);

	AttachWeaponToBody(OldEquippedWeapon);

	auto NewWeapon = Cast<AWeapon>(Inventory[IndexSlot]);
	NewWeapon->PlayEquipSound(true);

	EquipWeapon(NewWeapon);
}

void UInventoryComponent::AddWeapon(TObjectPtr<AWeapon> Weapon)
{
	if (Weapon == nullptr) return;

	auto WeaponType = Weapon->GetWeaponType();
	int32 WeaponSlot = GetSlotForWeapon(Weapon);

	if (Inventory[WeaponSlot] != nullptr)
	{
		//Existing Weapon in slot
		auto OldWeapon = Inventory[WeaponSlot];
		DropWeapon(OldWeapon);
		Inventory[WeaponSlot] = Weapon;

		if (InventoryCurrentSlot == WeaponSlot)
			EquipWeapon(Weapon);
		else
			AttachWeaponToBody(Weapon);

		Weapon->SetCharacter(Character);
	}
	else
	{
		//Add New Weapon in slot
		Inventory[WeaponSlot] = Weapon;
		Weapon->SetItemState(EItemState::EIS_PickedUp);
		Weapon->SetCharacter(Character);

		AttachWeaponToBody(Weapon);

		OnWeaponAddedEvent.Broadcast(WeaponSlot);
	}
}

bool UInventoryComponent::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	return EquippedWeapon->GetAmmo() > 0;
}

void UInventoryComponent::DeductAmmo(EAmmoType AmmoType, int32 AmountToDeduct)
{
	if (!AmmoMap.Contains(AmmoType)) return;
	AmmoMap[AmmoType] -= AmountToDeduct;
}

void UInventoryComponent::PickupAmmo(AAmmo* Ammo)
{
	if (Ammo == nullptr) return;

	//Check if AmmoMap has AmmoType
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount = AmmoMap[Ammo->GetAmmoType()];
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	//Ammo->Destroy();
}

bool UInventoryComponent::HasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	const auto AmmoType = EquippedWeapon->GetAmmoType();
	if (AmmoMap.Contains(AmmoType)) return AmmoMap[AmmoType] > 0;

	return false;
}

bool UInventoryComponent::HasAmmoOfType(EAmmoType AmmoType)
{
	if (AmmoMap.Contains(AmmoType)) return AmmoMap[AmmoType] > 0;

	return false;
}

void UInventoryComponent::OnReloadFinish()
{
	if (EquippedWeapon == nullptr) return;

	//Update AmmoMap
	const auto AmmoType = EquippedWeapon->GetAmmoType();
	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];
		
		//Space left in the mag of equipped weapon
		const int32 MagEmptySpace = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
		if (MagEmptySpace > CarriedAmmo)
		{
			//Reload the mag with all ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			//Fill the mag
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

FString UInventoryComponent::GetEquippedWeaponAmmo()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return FString();

	FString AmmoText = FString::FromInt(EquippedWeapon->GetAmmo());	
	FString CarriedText = FString::FromInt(AmmoMap[EquippedWeapon->GetAmmoType()]);

	return AmmoText + "/" + CarriedText;
}
