// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Enemy.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"
#include "BulletHitInterface.h"

#include "Perception/AISense_Hearing.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AWeapon::AWeapon() :
	FireMode(EFiringMode::EFM_SemiAuto),
	Ammo(0),
	MagCapacity(30),
	WeaponType(EWeaponType::EWT_SubmachineGun),
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(TEXT("ReloadSMG")),
	ClipBoneName(TEXT("smg_clip")),
	SlideDisplacement(0.0f),
	SlideDisplacementTime(0.2f),
	bMovingSlide(false),
	MaxSlideDisplacement(4.0f),
	MaxRecoilRotation(20.0f),
	ThrowWeaponTime(0.7f),
	RecoilPitch(-0.15f),
	bFalling(false),
	RecoilYaw(-0.15f)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (BoneToHide != FName(""))
		GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);

	if (MuzzleFlashEffect)
	{
		MuzzleFlashComp = UGameplayStatics::SpawnEmitterAttached(MuzzleFlashEffect,
			GetItemMesh(),
			TEXT("BarrelSocket"),
			FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector,
			EAttachLocation::KeepRelativeOffset, false, EPSCPoolMethod::None, false);
	}

	BarrelSocket = GetItemMesh()->GetSocketByName("BarrelSocket");
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	FString WeaponTablePath = TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'");
	UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

	if (WeaponTableObject)
	{
		FWeaponDataTable* WeaponDataRow = nullptr;

		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
			break;
		case EWeaponType::EWT_AssaultRifle:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
			break;
		case EWeaponType::EWT_Pistol:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
			break;
		case EWeaponType::EWT_Shotgun:
			WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Shotgun"), TEXT(""));
			break;
		}

		if (WeaponDataRow)
		{
			FireMode			= WeaponDataRow->FireMode;
			AmmoType			= WeaponDataRow->AmmoType;
			Ammo				= WeaponDataRow->WeaponAmmo;
			MagCapacity			= WeaponDataRow->MagCapacity;

			SetPickupSound(WeaponDataRow->PickupSound);
			SetEquipSound(WeaponDataRow->EquipSound);

			GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);

			SetItemName(WeaponDataRow->ItemName);

			SetItemIcon(WeaponDataRow->InventoryIcon);
			SetAmmoIcon(WeaponDataRow->AmmoIcon);

			SetMaterialInstance(WeaponDataRow->MaterialInstance);

			PreviousMaterialIndex = GetMaterialIndex();
			GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
			SetMaterialIndex(WeaponDataRow->MaterialIndex);

			SetClipBoneName(WeaponDataRow->ClipBoneName);

			SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);

			GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);

			CrosshairMiddle			= WeaponDataRow->CrosshairMiddle;
			CrosshairLeft			= WeaponDataRow->CrosshairLeft;
			CrosshairRight			= WeaponDataRow->CrosshairRight;
			CrosshairTop			= WeaponDataRow->CrosshairTop;
			CrosshairBottom			= WeaponDataRow->CrosshairBottom;

			CrosshairDefaultSpread	= WeaponDataRow->CrosshairDefaultSpread;
			AutoFireRate			= WeaponDataRow->AutoFireRate;
			MuzzleFlashEffect		= WeaponDataRow->MuzzleFlash;
			FireSound				= WeaponDataRow->FireSound;
			BoneToHide				= WeaponDataRow->BoneToHide;

			//Damage = WeaponDataRow->Damage;
			//HeadshotDamage = WeaponDataRow->HeadshotDamage;			
		}

		if (GetMaterialInstance())
		{
			SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
			GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());

			GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
			EnableGlowMaterial();
		}
	}

	FString WeaponRarityTablePath = TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/WeaponRarityDataTable.WeaponRarityDataTable'");
	UDataTable* WeaponRarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponRarityTablePath));

	if (WeaponRarityTableObject)
	{
		FString RowName;
		switch (WeaponType)
		{
		case EWeaponType::EWT_SubmachineGun:	RowName = "SMG_";		break;
		case EWeaponType::EWT_AssaultRifle:		RowName = "AR_";		break;
		case EWeaponType::EWT_Pistol:			RowName = "Pistol_";	break;
		case EWeaponType::EWT_Shotgun:			RowName = "Shotgun_";	break;
		}

		FWeaponRarityDataTable* WeaponRDataRow = nullptr;
		switch (GetItemRarity())
		{
		case EItemRarity::EIR_Damaged:			RowName += "Damaged";	break;
		case EItemRarity::EIR_Common:			RowName += "Common";	break;
		case EItemRarity::EIR_Uncommon:			RowName += "UnCommon";	break;
		case EItemRarity::EIR_Rare:				RowName += "Rare";		break;
		case EItemRarity::EIR_Legendary:		RowName += "Legendary";	break;
		}

		WeaponRDataRow = WeaponRarityTableObject->FindRow<FWeaponRarityDataTable>(FName(RowName), TEXT(""));
		if (WeaponRDataRow)
		{
			Damage				= WeaponRDataRow->DamageBody;
			HeadshotDamage		= WeaponRDataRow->DamageHeadshot;
			DamageMultiplier	= WeaponRDataRow->DamageMultiplier;
			Accuracy			= WeaponRDataRow->Accuracy;
			RecoilPitch			= WeaponRDataRow->RecoilPitch;
			RecoilYaw			= WeaponRDataRow->RecoilYaw;
		}
	}
}

bool AWeapon::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	if (Character == nullptr) return false;

	FVector2D ViewportSixe;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(ViewportSixe);

	FVector2D CrosshairLocation(ViewportSixe.X * 0.5f, ViewportSixe.Y * 0.5f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		//Trace from Crosshair World Location Outward
		float WeaponAccuracy = Character->bAiming ? Accuracy * 0.5f : Accuracy;

		const FVector Start = CrosshairWorldPosition;
		FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 5000.0f;
		End.X += FMath::FRandRange(-WeaponAccuracy, WeaponAccuracy);
		End.Y += FMath::FRandRange(-WeaponAccuracy, WeaponAccuracy);
		End.Z += FMath::FRandRange(-WeaponAccuracy, WeaponAccuracy);

		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

bool AWeapon::GetTraceEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;

	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult, OutBeamLocation);

	//Perform 2nd Trace from Gun Barrel	
	const FVector WeaponTraceStart = MuzzleSocketLocation;
	const FVector StartToEnd = OutBeamLocation - MuzzleSocketLocation;
	const FVector WeaponTraceEnd = OutBeamLocation + StartToEnd * 1.25f;
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);
	if (!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (bMovingSlide && SlideDisplacementCurve)
		UpdateSlideDisplacement();
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	//Here gun's Right Vector is pointing Forward!
	const FVector MeshForward{ GetItemMesh()->GetRightVector() };

	//Direction in which we throw the weapon
	float RandomRotation{ FMath::FRandRange(-20.0f, 20.0f) };

	FVector ImpulseDir = MeshForward.RotateAngleAxis(RandomRotation, FVector::UpVector);
	ImpulseDir *= 30.0f;

	GetItemMesh()->AddImpulse(ImpulseDir);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

	EnableGlowMaterial();
}

void AWeapon::Fire()
{
	if (Character == nullptr) return;
	
	if (Ammo > 0)
	{
		Ammo = FMath::Max(--Ammo, 0);

		switch (FireMode)
		{
		case EFiringMode::EFM_SemiAuto:
		case EFiringMode::EFM_Auto:				FireBullet();							break;
		case EFiringMode::EFM_Shotgun: for (int i = 0; i < 5; i++) { FireBullet(); }	break;
		}

		Character->CombatState = ECombatState::ECS_FireTimerInProgress;
		Character->PlayGunFireMontage();
		Character->StartCrosshairBulletFire();
		Character->AddControllerPitchInput(FMath::FRandRange(-RecoilPitch, -RecoilPitch));
		Character->AddControllerYawInput(FMath::FRandRange(-RecoilYaw, RecoilYaw));

		GetWorldTimerManager().SetTimer(FireTimerHandle, this, &AWeapon::OnFireTimerFinish, AutoFireRate);

		UAISense_Hearing::ReportNoiseEvent(this, Character->GetActorLocation(), 0.5f, this, 0.0f);

		if (MuzzleFlashComp) MuzzleFlashComp->Activate(true);
		if (FireSound) UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
		if (WeaponType == EWeaponType::EWT_Pistol) StartSlideTimer();
	}
}

void AWeapon::FireBullet()
{
	if (BarrelSocket == nullptr) return;

	const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetItemMesh());

	FHitResult TraceHitResult;
	
	FBulletHitData BulletHitData;
	BulletHitData.BodyDamage = Damage;
	BulletHitData.HeadshotDamage = HeadshotDamage;
	
	bool bBeamEnd = GetTraceEndLocation(SocketTransform.GetLocation(), TraceHitResult);
	if (bBeamEnd)
	{
		//Does Hit Actor implement Bullet Hit Interface?
		if (TraceHitResult.GetActor())
		{
			if (IBulletHitInterface* HitInterface = Cast<IBulletHitInterface>(TraceHitResult.GetActor()))
				HitInterface->BulletHit_Implementation(TraceHitResult, BulletHitData, Character, Character->GetController());
			else
			{
				if (BulletWallHitEffect != nullptr)
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletWallHitEffect, TraceHitResult.Location);
			}
		}
	}

	if (BeamParticleEffect != nullptr)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticleEffect, SocketTransform);
		if (Beam != nullptr)
			Beam->SetVectorParameter(FName("Target"), TraceHitResult.Location);
	}
}

void AWeapon::OnFireTimerFinish()
{
	if (Character == nullptr ) return;
	if (Character->CombatState == ECombatState::ECS_Stunned) return;

	Character->CombatState = ECombatState::ECS_Unoccupied;

	if (Ammo > 0)
	{
		if (Character->GetShooterController()->IsFireButtonHeld())
		{
			switch (FireMode)
			{
				case EFiringMode::EFM_SemiAuto:				break;
				case EFiringMode::EFM_Auto:
				case EFiringMode::EFM_Shotgun:	Fire();		break;
			}
		}
	}
	else
		Character->ReloadWeapon();
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	checkf(Ammo + Amount <= MagCapacity, TEXT("Attempted to reload with more than mag capacity"));
	Ammo += Amount;
}

void AWeapon::StartSlideTimer()
{
	bMovingSlide = true;
	GetWorldTimerManager().SetTimer(SlideTimer, FTimerDelegate::CreateLambda([&] 
		{
			bMovingSlide = false;
		}), SlideDisplacementTime, false);
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);

	StartPulseTimer();
}

void AWeapon::UpdateSlideDisplacement()
{
	const float ElaspedTime = GetWorldTimerManager().GetTimerElapsed(SlideTimer);
	const float CurveValue = SlideDisplacementCurve->GetFloatValue(ElaspedTime);

	SlideDisplacement = CurveValue * MaxSlideDisplacement;
	RecoilRotation = CurveValue * MaxRecoilRotation;
}