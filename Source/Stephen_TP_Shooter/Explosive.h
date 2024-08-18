// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IDamageable.h"
#include "Explosive.generated.h"

class UParticleSystem;
class USoundCue;
class USphereComponent;
class UStaticMeshComponent;
class URadialForceComponent;

UCLASS()
class STEPHEN_TP_SHOOTER_API AExplosive : public AActor, public IDamageable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Explode(AActor* Shooter, AController* ShooterController);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ExplosiveMesh;

	//Explosion when hit by bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ExplodeParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	USoundCue* ExplodeSound;

	//Used to determine what actor overlap during explosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	URadialForceComponent* ExplosionRadialForce;

	//Damage Amount for Explosive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ExplosionDamage;

	//Radius of Explosion to do Radial Damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ExplosionRadius;

	//Impulse Force of Explosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float ExplosionImpulse;

public:	
	virtual void ProcessDamageBasic_Implementation(const float& DamageAmount, AActor* Shooter, AController* ShooterController);
	//virtual void BulletHit_Implementation(FHitResult HitResult, const FBulletHitData& BulletHitData, AActor* Shooter, AController* ShooterController) override;
};