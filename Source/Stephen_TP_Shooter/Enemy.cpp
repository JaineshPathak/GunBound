// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "IShooterActions.h"
#include "EnemyController.h"
#include "EnemyManagerSubsystem.h"
#include "SoundsDataAsset.h"
#include "ShooterCharacter.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "ShooterGameState.h"
#include "Weapon.h"

#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/PlayerState.h"

#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

// Sets default values
AEnemy::AEnemy() :
	HealthBarDisplayTime(4.0f),
	bCanHitReact(true),
	HitReactTimeMin(0.5f),
	HitReactTimeMax(3.0f),
	HitNumberDestroyTime(1.5f),
	bCanMove(true),
	bStunned(false),
	StunChance(0.5f),
	bAttacking(false),
	BaseDamage(20.0f),
	LeftWeaponSocket(TEXT("FX_Trail_L_01")),
	RightWeaponSocket(TEXT("FX_Trail_R_01"))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create Combat Range Sphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatRangeSphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	LeftWeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponLeftBox"));
	LeftWeaponCollisionBox->SetupAttachment(GetMesh(), TEXT("LeftWeaponBone"));

	RightWeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponRightBox"));
	RightWeaponCollisionBox->SetupAttachment(GetMesh(), TEXT("RightWeaponBone"));

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("EnemyHealthComponent"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EnemyAudioComponent"));
	AudioComponent->SetupAttachment(GetRootComponent());

	AttackSectionNames[0] = TEXT("Attack_L");
	AttackSectionNames[1] = TEXT("Attack_R");
	AttackSectionNames[2] = TEXT("Attack_L_Fast");
	AttackSectionNames[3] = TEXT("Attack_R_Fast");
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(GetWorld()));
	ShooterGameState->OnSurvivalStartDelegate.AddDynamic(this, &AEnemy::OnMatchStart);
	ShooterGameState->OnSurvivalEndDelegate.AddDynamic(this, &AEnemy::OnMatchEnd);

	EnemyManager = ShooterGameState->GetEnemyManager();
	EnemyManager->OnEnemySpawnDelegate.Broadcast(this);

	//AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);
	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereOverlapBegin);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereOverlapEnd);

	//Bind Functions for Attacking Weapon Boxes
	LeftWeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);	
	
	//Set Weapon Boxes Collision Presets
	LeftWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightWeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	//Get the AI Controller
	const FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	const FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	if (EnemyController = Cast<AEnemyController>(GetController()))
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);
		EnemyController->RunBehaviorTree(BehaviorTree);
	}

	if (HealthComponent)
		HealthComponent->OnHealthDepletedEvent.AddDynamic(this, &AEnemy::OnDeath);
}

void AEnemy::ShowHealthBarEvent_Implementation()
{
	if(GetWorldTimerManager().IsTimerActive(HeathBarHideTimerHandle))
		GetWorldTimerManager().ClearTimer(HeathBarHideTimerHandle);
	
	GetWorldTimerManager().SetTimer(HeathBarHideTimerHandle, FTimerDelegate::CreateLambda([&]
		{
			HideHealthBarEvent();
		}), HealthBarDisplayTime, false);
}

void AEnemy::OnDeath(AActor* InstigatorActor, AController* InstigatorController)
{
	if (GetWorldTimerManager().IsTimerActive(HeathBarHideTimerHandle))
		GetWorldTimerManager().ClearTimer(HeathBarHideTimerHandle);

	HideHealthBarEvent();

	SetCanMove(false);
	SetAttacking(false);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesPhysicsBlendWeight(1.0f);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetMesh()->HideBoneByName(FName("weapon_l"), EPhysBodyOp::PBO_None);
	GetMesh()->HideBoneByName(FName("weapon_r"), EPhysBodyOp::PBO_None);

	PlayTheSound("DeathSound", false, false, 1.0f);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Dead"), true);
		EnemyController->StopMovement();
	}

	EnemyManager->OnEnemyDeathDelegate.Broadcast(this);

	GetWorldTimerManager().SetTimer(DestroyTimerHandle, FTimerDelegate::CreateLambda([&]
		{
			Destroy();
		}), 2.0f, false);
}

void AEnemy::OnMatchStart()
{
	if (!ShooterGameState || !ShooterGameState->GetShooterCharacter()) return;	

	DetectPlayer(ShooterGameState->GetShooterCharacter());
}

void AEnemy::OnMatchEnd(bool bVictory)
{
}

void AEnemy::PlayHitMontage(FName MontageSection, float PlayRate)
{
	if (bCanHitReact)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && HitMontage)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(MontageSection, HitMontage);
		}

		bCanHitReact = false;
		const float HitReactTime = FMath::FRandRange(HitReactTimeMin, HitReactTimeMax);
		GetWorldTimerManager().SetTimer(HitReactTimer, FTimerDelegate::CreateLambda([&]
			{
				bCanHitReact = true;
			}), HitReactTime, false);

		PlayTheSound("HitSound", true, false, 1.0f);
	}
}

FName AEnemy::GetAttackSectionName_Implementation()
{
	return AttackSectionNames[FMath::RandRange(0, 3)];
}

void AEnemy::PlayAttack_Implementation(FName MontageSection, float PlayRate)
{
	if (!bInAttackRange) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(MontageSection, AttackMontage);
	}

	PlayTheSound("AttackSound", false, false, 1.0f);
	PlayTheSound("AxeSwingSound", false, false, 1.0f);
}

bool AEnemy::IsTargetDead_Implementation()
{
	if (EnemyController == nullptr) return false;

	if (auto ShooterTarget = Cast<IShooterActions>(EnemyController->GetBlackboardComponent()->GetValueAsObject("Target")))
		return ShooterTarget->HasDied_Implementation();

	return false;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumberWidget, FVector Location)
{
	HitNumbersMap.Add(HitNumberWidget, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;

	HitNumberDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumberWidget);

	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumberWidget)
{
	HitNumbersMap.Remove(HitNumberWidget);
	HitNumberWidget->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPairs : HitNumbersMap)
	{
		UUserWidget* HitNumberWg = HitPairs.Key;
		const FVector HitNumberLoc = HitPairs.Value;

		FVector2D HitNumberScreenPos;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), HitNumberLoc, HitNumberScreenPos);

		HitNumberWg->SetPositionInViewport(HitNumberScreenPos);
	}
}

void AEnemy::SetStunned(bool IsStunned)
{
	if (HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	bStunned = IsStunned;
	
	if (bStunned) SetAttacking(false);

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);

	SetCanMove(true);
}

void AEnemy::SetAttacking(bool IsAttacking)
{
	if (HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	if (!bCanMove) SetCanMove(true);

	bAttacking = IsAttacking;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), bAttacking);
}

void AEnemy::SetCanMove(bool CanMove)
{
	if (HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	bCanMove = CanMove;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CanMove"), bCanMove);
}

void AEnemy::SetInvestigating(bool Investigate)
{
	if (HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsInvestigating"), Investigate);
}

void AEnemy::CombatRangeSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	if (auto Character = Cast<IShooterActions>(OtherActor))
	{
		SetCanMove(true);

		bInAttackRange = true;

		if (EnemyController)
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
	}
}

void AEnemy::CombatRangeSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;

	if (auto Character = Cast<IShooterActions>(OtherActor))
	{
		bInAttackRange = false;

		if (EnemyController)
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
	}
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto CharacterDamagable = Cast<IDamageable>(OtherActor))
		CharacterDamagable->ProcessDamageBasic_Implementation(BaseDamage, this, EnemyController);
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto CharacterDamagable = Cast<IDamageable>(OtherActor))
		CharacterDamagable->ProcessDamageBasic_Implementation(BaseDamage, this, EnemyController);
}

void AEnemy::DetectPlayer(AActor* OtherActor)
{
	if (OtherActor == nullptr) return;
	if (EnemyController == nullptr) return;
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;

	if (auto Character = Cast<IShooterActions>(OtherActor))
	{
		SetInvestigating(false);

		//Set value of 'Target' Blackboard Key
		if (!bGreetedPlayer)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), OtherActor);
			bGreetedPlayer = true;

			if (FMath::FRand() <= 0.6f)
			{
				PlayTheSound("GreetSound", true, false, 1.0f);

				UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
				if (AnimInst && TauntMontage)
					AnimInst->Montage_Play(TauntMontage);
			}
			else
				SetCanMove(true);
		}
	}
}

void AEnemy::InvestigateNoiseLocation(const FVector& NoiseLocation)
{
	if (HealthComponent && HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;
	if (FMath::IsNearlyEqual(NoiseLocation.Size(), 0.01)) return;
	if (EnemyController->GetBlackboardComponent()->GetValueAsObject("Target")) return;

	SetInvestigating(true);
	SetCanMove(true);

	EnemyController->GetBlackboardComponent()->SetValueAsVector("InvestigateLocation", NoiseLocation);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();
}

void AEnemy::PlayTheSound(const FString& SoundName, bool bFadeIn, bool bFadeOut, float VolumeMultiplier)
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

FVector AEnemy::GetEnemyTargetLocation() const
{
	if (!bGreetedPlayer || EnemyController == nullptr) return FVector::ZeroVector;

	if (auto Character = Cast<IShooterActions>(EnemyController->GetBlackboardComponent()->GetValueAsObject(TEXT("Target"))))
		return Character->GetShooterLocation_Implementation();

	return FVector::ZeroVector;
}

void AEnemy::ProcessDamage_Implementation(const FHitResult& HitResult, const float& DamageAmount, AActor* Shooter, AController* ShooterController)
{
	PlayTheSound("BulletHitSound", false, false, 2.0f);

	if (ImpactParticles)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.0f), true);

	bool bHeadshot = (HitResult.BoneName.ToString() == HeadBone);
	ShowHitNumber((int32)DamageAmount, HitResult.Location, bHeadshot);

	UGameplayStatics::ApplyDamage(this, DamageAmount, ShooterController, Shooter, UDamageType::StaticClass());
}

void AEnemy::JumpToDestination_Implementation(FVector Destination)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;

	const FVector StartPos = GetActorLocation();
	FVector EndPos = Destination;
	EndPos.Z += 250.0f;

	FVector LaunchVelocity = FVector::ZeroVector;
	UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, LaunchVelocity, StartPos, EndPos);

	PlayTheSound("JumpSound", true, false, 1.0f);

	LaunchCharacter(LaunchVelocity, true, true);
}

void AEnemy::ApplyStun_Implementation()
{
	const float StunningChance = FMath::FRandRange(0.0f, 1.0f);
	if (StunningChance <= StunChance)
	{
		SetStunnedStatus_Implementation(true);
		PlayHitMontage("HitReactFront");
		PlayTheSound("StunSound", true, false, 1.0f);
	}
}

void AEnemy::SetStunnedStatus_Implementation(bool IsStunned)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	bStunned = IsStunned;

	if (bStunned) SetAttackingStatus_Implementation(false);

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), bStunned);

	SetCanMoveStatus_Implementation(true);
}

void AEnemy::SetAttackingStatus_Implementation(bool IsAttacking)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	if (!bCanMove) SetCanMoveStatus_Implementation(true);

	bAttacking = IsAttacking;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Attacking"), bAttacking);
}

void AEnemy::SetCanMoveStatus_Implementation(bool CanMove)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	bCanMove = CanMove;
	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("CanMove"), bCanMove);
}

void AEnemy::SetInvestigatingStatus_Implementation(bool Investigate)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return;
	if (EnemyController == nullptr) return;

	EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("IsInvestigating"), Investigate);
}

void AEnemy::SetWeaponLeftStatus_Implementation(bool Activate)
{
	if (LeftWeaponCollisionBox == nullptr) return;

	LeftWeaponCollisionBox->SetCollisionEnabled(Activate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void AEnemy::SetWeaponRightStatus_Implementation(bool Activate)
{
	if (RightWeaponCollisionBox == nullptr) return;

	RightWeaponCollisionBox->SetCollisionEnabled(Activate ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

float AEnemy::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (HealthComponent == nullptr || HealthComponent->IsDead()) return DamageAmount;

	HealthComponent->OnTakeDamage(DamageAmount, DamageCauser, EventInstigator);

	if (auto ShooterCharacter = Cast<IShooterActions>(DamageCauser))
	{
		float NewScore = (0.75f * DamageAmount) + ((ShooterGameState != nullptr) ? ShooterGameState->GetCurrentWave() : 1.0f);
		ShooterCharacter->AddScore_Implementation(NewScore);
	}

	if(EnemyManager && EnemyManager->OnEnemyHitDelegate.IsBound())
		EnemyManager->OnEnemyHitDelegate.Broadcast(this);

	if (EnemyController)
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), DamageCauser);

	if (!bGreetedPlayer) bGreetedPlayer = true;

	SetCanMove(true);
	SetInvestigating(false);

	if (HealthComponent->IsDead()) return DamageAmount;

	ShowHealthBarEvent();
	ApplyStun_Implementation();

	return DamageAmount;
}