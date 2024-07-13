// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Perception/AISense_Hearing.h"

// Sets default values
AGrenade::AGrenade() :
	ExplosionDamage(100.0f),
	ExplosionTime(3.0f),
	ExplosionRadius(200.0f),
	ExplosionImpulse(1000.0f),
	ThrowForce(1000.0f),
	ThrowUpForce(100.0f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	SetRootComponent(GrenadeMesh);

	ExplosionRadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("ExplosionRadialForce"));
	ExplosionRadialForce->SetupAttachment(GetRootComponent());
	ExplosionRadialForce->bIgnoreOwningActor = true;
	ExplosionRadialForce->bImpulseVelChange = false;
	ExplosionRadialForce->ImpulseStrength = ExplosionImpulse;
	ExplosionRadialForce->Radius = ExplosionRadius;
}

void AGrenade::InitGrenade()
{
	if (GetInstigator() == nullptr) return;

	//ShooterInstigator = PlayerInstigator;
	//ShooterController = Cast<AShooterPlayerController>(PlayerInstigator->GetController());

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetInstigator());

	if (GrenadeMesh && ShooterCharacter)
	{
		const FVector ThrowDir = ShooterCharacter->GetFollowCamera()->GetForwardVector() * ThrowForce + FVector::UpVector * ThrowUpForce;

		//GrenadeMesh->IgnoreActorWhenMoving(ShooterCharacter, false);
		GrenadeMesh->SetSimulatePhysics(true);
		GrenadeMesh->SetEnableGravity(true);
		GrenadeMesh->AddImpulse(ThrowDir, NAME_None, true);
		
		GetWorldTimerManager().SetTimer(ExplosionTimerHandle, this, &AGrenade::ExplosionTimerFinish, ExplosionTime);
	}
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
}

void AGrenade::ExplosionTimerFinish()
{
	Explode();
}

void AGrenade::Explode()
{
	if (ExplodeSound)
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	if (ExplodeParticles)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles, GetActorLocation(), FRotator::ZeroRotator, true);

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, UDamageType::StaticClass(), IgnoredActors, GetInstigator(), GetInstigatorController(), true);
	
	ExplosionRadialForce->FireImpulse();

	UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), 0.5f, GetInstigator(), 0.0f);

	Destroy();
}

