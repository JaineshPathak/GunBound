// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

class UParticleSystem;
class USoundCue;
class USphereComponent;
class UStaticMeshComponent;
class AShooterCharacter;
class AShooterPlayerController;
class URadialForceComponent;

UCLASS()
class STEPHEN_TP_SHOOTER_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenade();

	void InitGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void ExplosionTimerFinish();
	void Explode();

private:
	//Static Mesh of Grenade
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* GrenadeMesh;

	//Explosion FX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ExplodeParticles;

	//Explosion Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	USoundCue* ExplodeSound;

	//Used to determine what actor overlap during explosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	URadialForceComponent* ExplosionRadialForce;

	//Damage Amount for Exploding
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	float ExplosionDamage;

	//Time before explosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	float ExplosionTime;

	//Radius of Explosion to do Radial Damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	float ExplosionRadius;

	//Impulse Force of Explosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	float ExplosionImpulse;

	//Forward Throw Force of Grenade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	float ThrowForce;

	//Upward Throw Force of Grenade
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenade", meta = (AllowPrivateAccess = "true"))
	float ThrowUpForce;

	FTimerHandle ExplosionTimerHandle;

	//AShooterCharacter* ShooterInstigator;
	//AShooterPlayerController* ShooterController;
};
