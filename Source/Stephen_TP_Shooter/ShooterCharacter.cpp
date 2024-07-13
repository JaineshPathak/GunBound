// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"
#include "Item.h"
#include "Weapon.h"
#include "Ammo.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "Grenade.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "SoundsDataAsset.h"
#include "BulletHitInterface.h"
#include "Stephen_TP_Shooter.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "DrawDebugHelpers.h"
#include "Perception/AISense_Hearing.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2),

	//Aiming Flag
	bAiming(false),

	//Camera FOV Values
	CameraDefaultFOV(0.0f),
	CameraAimingFOV(45.0f),
	CameraCurrentFOV(0.0f),
	AimShootFOVThreshold(20.0f),
	AimingInterpSpeed(20.0f),

	//Crosshair Spread Factors
	CrosshairSpreadMultiplier(0.0f),
	CrosshairVelocityFactor(0.0f),
	CrosshairInAirFactor(0.0f),
	CrosshairAimFactor(0.0f),
	CrosshairShootingFactor(0.0f),

	//Bullet Fire Timer Vars
	ShootTimeDuration(0.05f),
	bFiringBullet(false),

	//Item Variables
	bShouldTraceForItems(false),

	CameraInterpDistance(150.0f),
	CameraInterpElevation(40.0f),

	//Combat State vars
	CombatState(ECombatState::ECS_Unoccupied),

	//Crouch Vars
	bCrouching(false),

	BaseMovementSpeed(650.0f),
	CrouchMovementSpeed(300.0f),

	StandingCapsuleHalfHeight(90.0f),
	CrouchingCapsuleHalfHeight(45.0f),

	BaseGroundFriction(2.0f),
	CrouchingGroundFriction(100.0f),

	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	
	//Icon Animation Property
	HighlightedSlot(-1),

	StunChance(1.0f),

	ThrowGrenadeTauntTime(5.0f),
	KillTauntTime(10.0f),

	bTauntInGrenadeThrow(true),
	bCanKillTaunt(true)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Camera Boom (pulls in towards the character if there is a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 240.f;		//Camera follows at the distance behind the character
	CameraBoom->bUsePawnControlRotation = true;//Rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.0f, 70.0f, 80.0f);

	//Create follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//Dont rotate when the controller rotates.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Config character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;	//rotates in direction of input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	//Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpolationComponent"));
	WeaponInterpComp->SetupAttachment(FollowCamera);

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent1"));
	InterpComp1->SetupAttachment(FollowCamera);

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent2"));
	InterpComp2->SetupAttachment(FollowCamera);

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent3"));
	InterpComp3->SetupAttachment(FollowCamera);

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent4"));
	InterpComp4->SetupAttachment(FollowCamera);

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent5"));
	InterpComp5->SetupAttachment(FollowCamera);

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpolationComponent6"));
	InterpComp6->SetupAttachment(FollowCamera);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("CharacterHealthComponent"));
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("CharacterInventoryComponent"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CharacterAudioComponent"));
	AudioComponent->SetupAttachment(GetRootComponent());

	ShooterPlayerController = Cast<AShooterPlayerController>(GetController());
}

float AShooterCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return DamageAmount;

	HealthComponent->OnTakeDamage(DamageAmount, DamageCauser, EventInstigator);

	PlayTheSound("HitHurtSound", true, false, 1.0f);
	PlayTheSound("HitSound", true, false, 5.0f);

	DetermineStunChance();

	return DamageAmount;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();	

	if (FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	
	InitInterpLocations();

	if (HealthComponent)
		HealthComponent->OnHealthDepletedEvent.AddDynamic(this, &AShooterCharacter::OnDeath);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HealthComponent && HealthComponent->IsDead()) return;

	//Handle Interpolation when aiming
	CameraInterpAim(DeltaTime);

	//Calc Crosshair Spread Multiplier
	CalcCrosshairSpread(DeltaTime);

	//Check Overlapped Item count then trace for items
	TraceForItems();

	//Interp Capsule Component based on standing/crouching
	InterpCapsuleHeights(DeltaTime);
}

void AShooterCharacter::FireWeapon()
{
	if (!bCanFireWeapon) return; 
	
	InventoryComponent->GetEquippedWeapon()->Fire();
}

void AShooterCharacter::AimingPressed()
{
	if (CombatState != ECombatState::ECS_Reloading && 
		CombatState != ECombatState::ECS_Equipping &&
		CombatState != ECombatState::ECS_Stunned)
		Aim();
}

void AShooterCharacter::AimingReleased() 
{ 
	if(!GetShooterController()->IsFireButtonHeld()) StopAiming(); 
}

void AShooterCharacter::CameraInterpAim(float DeltaTime)
{
	const float TargetFOV = bAiming ? CameraAimingFOV : CameraDefaultFOV;

	CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, TargetFOV, DeltaTime, AimingInterpSpeed);

	if (!bCanFireWeapon && bAiming)
	{
		const float TargetFOVDistance = TargetFOV - CameraCurrentFOV;
		if (FMath::Square(TargetFOVDistance) <= AimShootFOVThreshold)
		{
			bCanFireWeapon = true;
			if(GetShooterController()->IsFireButtonHeld())
				InventoryComponent->GetEquippedWeapon()->Fire();
		}
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::CalcCrosshairSpread(float DeltaTime)
{
	if (InventoryComponent == nullptr || InventoryComponent->GetEquippedWeapon() == nullptr) return;

	auto& CurrentWeapon = *InventoryComponent->GetEquippedWeapon();

	FVector2D WalkSpeedRange{ 0.0f, 600.0f };
	FVector2D VelocityMultiplierRange{ 0.0f, 1.0f };
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	//In Air Crosshair Handling
	const float InAirTarget = GetCharacterMovement()->IsFalling() ? 2.25f : 0.0f;
	const float InAirTargetSpeed = GetCharacterMovement()->IsFalling() ? 2.25f : 30.0f;
	CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, InAirTarget, DeltaTime, InAirTargetSpeed);

	//Aiming Crosshair Handling
	const float AimFactorTarget = bAiming ? 0.5f : 0.0f;
	CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, AimFactorTarget, DeltaTime, 30.0f);

	//When Firing
	const float ShootFactorTarget = bFiringBullet ? 0.3f : 0.0f;
	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, ShootFactorTarget, DeltaTime, 60.0f);

	CrosshairSpreadMultiplier = CurrentWeapon.GetCrosshairDefaultSpread() + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, FTimerDelegate::CreateLambda([&]
		{
			bFiringBullet = false;
		}), ShootTimeDuration, false);
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSixe;
	if (GEngine && GEngine->GameViewport)
		GEngine->GameViewport->GetViewportSize(ViewportSixe);

	FVector2D CrosshairLocation(ViewportSixe.X * 0.5f, ViewportSixe.Y * 0.5f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);
	if (bScreenToWorld)
	{
		const FVector Start = CrosshairWorldPosition;
		FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 5000.0f;
		
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

void AShooterCharacter::TraceForItems()
{
	if (HealthComponent->IsDead()) return;

	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshair(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			/*if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					//Not currently highlighting a slot
					HighlightInventorySlot();
				}
			}
			else
			{
				//Is Slot being highlighted?
				if (HighlightedSlot != -1)
				{
					//Unhighlight the Inventory slot
					UnHighlightInventorySlot();
				}
			}*/
			
			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
				TraceHitItem = nullptr;

			if (TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				//m_TraceHitItem->SetCharacterInventoryFull(Inventory.Num() >= InventoryCapacity);
			}

			//we hit AItem last frame
			if (TracedItemLastFrame)
			{
				//Its a different AItem, so turn off visibility of last traced Item
				if (TraceHitItem != TracedItemLastFrame)
				{
					TracedItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TracedItemLastFrame->DisableCustomDepth();
				}
			}

			//Store a reference to HitItem last frame
			TracedItemLastFrame = TraceHitItem;
		}
	}
	else if (TracedItemLastFrame)
	{
		//We moved out of Overlapping Sphere
		TracedItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TracedItemLastFrame->DisableCustomDepth();
	}
}

void AShooterCharacter::Interact()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (TraceHitItem == nullptr || !TraceHitItem->IsInteractable()) return;

	if (bAiming) StopAiming();

	//TraceHitItem->SetCharacter(this);
	//TraceHitItem->FinishInterping();
	TraceHitItem->StartItemCurve(this, true);

	CombatState = ECombatState::ECS_PickingUp;

	if (InventoryComponent->GetEquippedWeapon())
		InventoryComponent->GetEquippedWeapon()->SetItemState(EItemState::EIS_InActive);

	PlayAnAnimMontage(PickupMontage, FName("PickupMontage"));
}

void AShooterCharacter::PlayKillTauntSound()
{
	if (!bCanKillTaunt) return;

	PlayTheSound("KillTauntSound", false, false, 1.0f);

	bCanKillTaunt = false;
	GetWorldTimerManager().SetTimer(KillTauntTimer, FTimerDelegate::CreateLambda([&] { bCanKillTaunt = true; }), KillTauntTime, false);
}

EPhysicalSurface AShooterCharacter::GetFootstepSurfaceType()
{
	FHitResult FootHitResult;

	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.0f, 0.0f, -400.0f);
	
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(FootHitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(FootHitResult.PhysMaterial.Get());
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (GetShooterController()->IsAiming()) Aim();
}

void AShooterCharacter::OnDeath(AActor* InstigatorActor, AController* InstigatorController)
{
	auto EnemyController = Cast<AEnemyController>(InstigatorController);
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), nullptr);
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("TargetDead"), true);
	}

	if (bAiming) StopAiming();

	PlayTheSound("DeathSound", true, false, 1.0f);

	if (InventoryComponent->GetEquippedWeapon())
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		InventoryComponent->GetEquippedWeapon()->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		InventoryComponent->GetEquippedWeapon()->SetItemState(EItemState::EIS_Falling);
		InventoryComponent->GetEquippedWeapon()->ThrowWeapon();
	}

	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	/*const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
		HandSocket->Detach(m_EquippedWeapon, GetMesh());*/

	AShooterPlayerController* PC = Cast<AShooterPlayerController>(GetController());
	if (PC)
	{
		CameraBoom->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		CameraBoom->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform);
		CameraBoom->SocketOffset = FVector::ZeroVector;

		PC->StopMovement();
		GetCharacterMovement()->DisableMovement();

		DisableInput(PC);
	}

	OnShooterDeath.Broadcast();
}

void AShooterCharacter::ThrowGrenade()
{
	const auto GrenadeAmmoType = EAmmoType::EAT_Grenade;

	if (HealthComponent->IsDead()) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (!InventoryComponent->HasAmmoOfType(GrenadeAmmoType)) return;

	if (bAiming) StopAiming();

	CombatState = ECombatState::ECS_Throwing;

	PlayAnAnimMontage(ThrowGrenadeMontage, FName("StartThrow"));

	if (bTauntInGrenadeThrow)
	{
		PlayTheSound("ThrowGrenadeSound", true, false, 1.0f);

		bTauntInGrenadeThrow = false;
		GetWorldTimerManager().SetTimer(ThrowGrenadeTauntTimer, FTimerDelegate::CreateLambda([&]
			{
				bTauntInGrenadeThrow = true;
			}), ThrowGrenadeTauntTime, false);
	}
	
	if (InventoryComponent->GetEquippedWeapon())
		InventoryComponent->GetEquippedWeapon()->SetItemState(EItemState::EIS_InActive);

	//Spawn the Grenade
	InventoryComponent->DeductAmmo(GrenadeAmmoType, 1);

	FActorSpawnParameters GrenadeSpawnParams;
	GrenadeSpawnParams.Instigator = this;

	const FVector GrenadeSpawnLoc = GetMesh()->GetSocketLocation(FName("RightHandSocket"));
	if (DefaultGrenadeClass)
	{
		AGrenade* GrenadeProj = GetWorld()->SpawnActor<AGrenade>(DefaultGrenadeClass, GrenadeSpawnLoc, GetActorRotation(), GrenadeSpawnParams);
		if (GrenadeProj)
			GrenadeProj->InitGrenade();
	}
}

void AShooterCharacter::DetermineStunChance()
{
	const float StunningChance = FMath::FRandRange(0.0f, 1.0f);
	if (StunningChance <= StunChance) Stun();
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CamWorldLocation{ FollowCamera->GetComponentLocation() };
	const FVector CamForward{ FollowCamera->GetForwardVector() };

	return CamWorldLocation + CamForward * CameraInterpDistance + FVector(0.0f, 0.0f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if (Item == nullptr) return;
	if (InventoryComponent == nullptr) return;
	
	if (AWeapon* Weapon = Cast<AWeapon>(Item)) InventoryComponent->AddWeapon(Weapon);
	if (AAmmo* Ammo = Cast<AAmmo>(Item)) InventoryComponent->PickupAmmo(Ammo);
}

void AShooterCharacter::PlayGunFireMontage() { PlayAnAnimMontage(HipFireMontage, FName("StartFire")); }

void AShooterCharacter::PlayAnAnimMontage(TObjectPtr<UAnimMontage> MontageToPlay, FName MontageSectionName)
{
	if (GetMesh() == nullptr || MontageToPlay == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(MontageToPlay);
		if (MontageSectionName != NAME_None)
			AnimInstance->Montage_JumpToSection(MontageSectionName);
	}
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	if (InventoryComponent->GetEquippedWeapon() == nullptr) return;

	if (InventoryComponent->HasAmmo() && !InventoryComponent->GetEquippedWeapon()->ClipIsFull())
	{
		if (bAiming) StopAiming();

		CombatState = ECombatState::ECS_Reloading;

		PlayAnAnimMontage(ReloadMontage, InventoryComponent->GetEquippedWeapon()->GetReloadMontageSection());
	}
}

void AShooterCharacter::GrabClip()
{
	if (InventoryComponent->GetEquippedWeapon() == nullptr || InventoryComponent->GetEquippedWeapon()->GetClipBoneName() == NAME_None) return;
	if (HandSceneComponent == nullptr) return;

	auto& CurrentWeapon = *InventoryComponent->GetEquippedWeapon();
	auto& CurrentWeaponMesh = *CurrentWeapon.GetItemMesh();

	//Index for the Clip Bone
	int32 ClipBoneIndex{ CurrentWeaponMesh.GetBoneIndex(CurrentWeapon.GetClipBoneName()) };

	//Store the transform
	ClipTransform = CurrentWeaponMesh.GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	CurrentWeapon.SetMovingClip(true);

	//UE_LOG(LogTemp, Warning, TEXT("Moving Clip Val: %s"), m_EquippedWeapon->GetMovingClip() ? TEXT("True") : TEXT("False"));
	/*if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Moving Clip Val: %s"), CurrentWeapon.GetMovingClip() ? TEXT("true") : TEXT("false")));*/
}

void AShooterCharacter::ReleaseClip()
{
	if (!InventoryComponent || InventoryComponent->GetEquippedWeapon() == nullptr) return;
	
	InventoryComponent->GetEquippedWeapon()->SetMovingClip(false);

	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	HandSceneComponent->DetachFromComponent(DetachmentRules);
	HandSceneComponent->SetWorldTransform(FTransform::Identity);

	/*if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("Moving Clip Val: %s"), InventoryComponent->GetEquippedWeapon()->GetMovingClip() ? TEXT("true") : TEXT("false")));*/
}

void AShooterCharacter::CrouchPressed()
{
	if (!GetCharacterMovement()->IsFalling())
		bCrouching = !bCrouching;

	GetCharacterMovement()->MaxWalkSpeed = (bCrouching) ? CrouchMovementSpeed : (bAiming) ? CrouchMovementSpeed : BaseMovementSpeed;
	GetCharacterMovement()->GroundFriction = (bCrouching) ? CrouchingGroundFriction : (bAiming) ? CrouchingGroundFriction : BaseGroundFriction;

	/*if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		if (bAiming)
		{
			GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
			GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
			GetCharacterMovement()->GroundFriction = BaseGroundFriction;
		}
	}*/
}

void AShooterCharacter::FiringPressed()
{
	if (HealthComponent->IsDead()) return;
	if (InventoryComponent == nullptr || InventoryComponent->GetEquippedWeapon() == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	bCanFireWeapon = false;

	Aim();
	FireWeapon();
}

void AShooterCharacter::FiringReleased()
{
	if (bAiming && !GetShooterController()->IsAiming()) StopAiming();
}

void AShooterCharacter::Jump()
{
	if (HealthComponent && HealthComponent->IsDead()) return;

	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
	else
	{
		if (!GetCharacterMovement()->IsFalling())
			PlayTheSound("JumpSound", true, false, 1.0f);

		ACharacter::Jump();
	}
}

void AShooterCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;

	PlayTheSound("LandSound", true, false, 1.0f);
}

void AShooterCharacter::InterpCapsuleHeights(float DeltaTime)
{
	float TargetHeight{};
	if (bCrouching)
		TargetHeight = CrouchingCapsuleHalfHeight;
	else
		TargetHeight = StandingCapsuleHalfHeight;

	const float InterpHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetHeight, DeltaTime, 20.0f);
	
	//Negative value if crouching; Positive value if standing
	const float DeltaCapsuleHalfHeight = InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector MeshOffset(0.0f, 0.0f, -DeltaCapsuleHalfHeight);
	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::InitInterpLocations()
{
	FInterpLocation WeaponLocation{WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;
	if (Index >= InterpLocations.Num()) return;

	InterpLocations[Index].ItemCount += Amount;
}

void AShooterCharacter::SwitchToWeapon(int32 NewItemIndex)
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	bool ShouldPlayAnim = false;
	InventoryComponent->SwitchToWeapon(NewItemIndex, ShouldPlayAnim);

	if (ShouldPlayAnim)
	{
		if (bAiming) StopAiming();

		CombatState = ECombatState::ECS_Equipping;
		PlayAnAnimMontage(EquipMontage, FName("Equip"));
	}
}

void AShooterCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	//Update the combat state
	CombatState = ECombatState::ECS_Unoccupied;

	if (GetShooterController()->IsAiming()) Aim();
	
	if (InventoryComponent->GetEquippedWeapon() == nullptr) return;

	InventoryComponent->OnReloadFinish();
}

void AShooterCharacter::FinishEquipping()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;

	if (GetShooterController()->IsAiming()) Aim();
}

void AShooterCharacter::FinishThrowingGrenade()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;

	if (GetShooterController()->IsAiming()) Aim();
	
	if (InventoryComponent->GetEquippedWeapon())
	{
		InventoryComponent->GetEquippedWeapon()->SetItemState(EItemState::EIS_Equipped);
		InventoryComponent->GetEquippedWeapon()->PlayEquipSound(true);

		CombatState = ECombatState::ECS_Equipping;
		PlayAnAnimMontage(EquipMontage, FName("Equip"));
	}
}

void AShooterCharacter::FinishPickup()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;
	
	if (GetShooterController()->IsAiming()) Aim();

	if (InventoryComponent->GetEquippedWeapon())
		InventoryComponent->GetEquippedWeapon()->SetItemState(EItemState::EIS_Equipped);
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

TObjectPtr<AShooterPlayerController> AShooterCharacter::GetShooterController()
{
	if (ShooterPlayerController == nullptr) 
		ShooterPlayerController = Cast<AShooterPlayerController>(GetController()); 
	
	return ShooterPlayerController;
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
		return InterpLocations[Index];

	return FInterpLocation();
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

void AShooterCharacter::Stun()
{
	CombatState = ECombatState::ECS_Stunned;

	PlayAnAnimMontage(HitReactMontage);
}

void AShooterCharacter::PlayTheSound(const FString& SoundName, bool bFadeIn, bool bFadeOut, float VolumeMultiplier)
{
	if (AudioComponent == nullptr || SoundsDataAsset == nullptr) return;
	if (!SoundsDataAsset->AudioMap.Contains(SoundName) || SoundsDataAsset->AudioMap[SoundName] == nullptr) return;

	auto& Sound = SoundsDataAsset->AudioMap[SoundName];
	AudioComponent->SetSound(Sound);
	AudioComponent->SetVolumeMultiplier(VolumeMultiplier);

	if (bFadeIn)
		AudioComponent->FadeIn(0.5f);
	else if (bFadeOut)
		AudioComponent->FadeOut(0.5f, 1.0f);
	else
		AudioComponent->Play();
}
