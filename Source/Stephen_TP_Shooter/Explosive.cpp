// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "GameFramework/Character.h"
#include "Perception/AISense_Hearing.h"

// Sets default values
AExplosive::AExplosive() : 
	ExplosionDamage(100.0f),
	ExplosionRadius(200.0f),
	ExplosionImpulse(1000.0f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExplosiveMesh"));
	SetRootComponent(ExplosiveMesh);

	ExplosionRadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("ExplosionRadialForce"));
	ExplosionRadialForce->SetupAttachment(GetRootComponent());
	ExplosionRadialForce->bIgnoreOwningActor = true;
	ExplosionRadialForce->bImpulseVelChange = false;
	ExplosionRadialForce->ImpulseStrength = ExplosionImpulse;
	ExplosionRadialForce->Radius = ExplosionRadius;
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
}

void AExplosive::Explode(AActor* Shooter, AController* ShooterController)
{
	if (ExplodeSound)
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());

	if (ExplodeParticles)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles, GetActorLocation(), FRotator(0.0f), true);

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	UGameplayStatics::ApplyRadialDamage(GetWorld(), ExplosionDamage, GetActorLocation(), ExplosionRadius, UDamageType::StaticClass(), IgnoredActors, Shooter, ShooterController, true);

	ExplosionRadialForce->FireImpulse();

	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, Shooter, 0.0f);

	Destroy();
}

void AExplosive::ProcessDamageBasic_Implementation(const float& DamageAmount, AActor* Shooter, AController* ShooterController)
{
	Explode(Shooter, ShooterController);
}
