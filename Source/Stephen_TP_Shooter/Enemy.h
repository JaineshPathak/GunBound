// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IPawnActions.h"
#include "IEnemyPawnActions.h"
#include "IDamageable.h"
#include "Enemy.generated.h"

class UParticleSystem;
class USoundCue;
class UBehaviorTree;
class USphereComponent;
class UBoxComponent;
class UAudioComponent;

class AEnemyController;
class AShooterCharacter;
class UHealthComponent;
class USoundsDataAsset;
class AShooterGameState;
class UEnemyManagerSubsystem;

UCLASS()
class STEPHEN_TP_SHOOTER_API AEnemy : public ACharacter, public IDamageable, public IPawnActions, public IEnemyPawnActions
{
	GENERATED_BODY()

public:
	AEnemy();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	
	virtual void ProcessDamage_Implementation(const FHitResult& HitResult, const float& DamageAmount, AActor* Shooter, AController* ShooterController);
	
	virtual void JumpToDestination_Implementation(FVector Destination) override;
	virtual void ApplyStun_Implementation() override;

	virtual void SetStunnedStatus_Implementation(bool IsStunned) override;
	virtual void SetAttackingStatus_Implementation(bool IsAttacking) override;
	virtual void SetCanMoveStatus_Implementation(bool CanMove) override;
	virtual void SetInvestigatingStatus_Implementation(bool Investigate) override;
	virtual void SetWeaponLeftStatus_Implementation(bool Activate) override;
	virtual void SetWeaponRightStatus_Implementation(bool Activate) override;
	virtual FName GetAttackSectionName_Implementation() override;
	virtual void PlayAttack_Implementation(FName MontageSection, float PlayRate = 1.0f) override;
	virtual bool IsTargetDead_Implementation() override;

	void UpdateHitNumbers();

	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBarEvent();

	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBarEvent();

	UFUNCTION()
	void OnMatchStart();

	UFUNCTION()
	void OnMatchEnd(bool bVictory);

	UFUNCTION(BlueprintCallable)
	void PlayHitMontage(FName MontageSection, float PlayRate = 1.0f);

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumberWidget, FVector Location);

	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumberWidget);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool IsStunned);

	UFUNCTION(BlueprintCallable)
	void SetAttacking(bool IsAttacking);

	UFUNCTION(BlueprintCallable)
	void SetCanMove(bool CanMove);

	UFUNCTION(BlueprintCallable)
	void SetInvestigating(bool Investigate);

	UFUNCTION()
	void CombatRangeSphereOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatRangeSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnDeath(AActor* InstigatorActor, AController* InstigatorController);

	UFUNCTION(BlueprintCallable)
	void InvestigateNoiseLocation(const FVector& NoiseLocation);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	AShooterGameState* ShooterGameState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events", meta = (AllowPrivateAccess = "true"))
	UEnemyManagerSubsystem* EnemyManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Particles", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> ImpactParticles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat - Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat - Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundsDataAsset> SoundsDataAsset;

	//Name of the Head bone for Headshots
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FString HeadBone;

	//Time to display Health Bar once shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bCanHitReact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;

	//Map to store Hit number widgets and their Hit Locations
	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumbersMap;

	//Time before a HitNumberBP removed from screen
	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float HitNumberDestroyTime;

	//Behavior Tree for the AI Character
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	//(MakeEditWidget = "true") - Makes a modifiable Widget. Makes the Enemy move to Patrol Point
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	//2nd Patrol Point to make Enemy Move To
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bGreetedPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Flags", meta = (AllowPrivateAccess = "true"))
	bool bCanMove;

	//True when playing the Hit Animation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat - Flags", meta = (AllowPrivateAccess = "true"))
	bool bStunned;

	//Chance of being stunned, 0 = No chance, 1 = 100% Chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Flags", meta = (AllowPrivateAccess = "true"))
	float StunChance;

	//True, when in Attack Range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Flags", meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;

	//True, when playing Attack Animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Flags", meta = (AllowPrivateAccess = "true"))
	bool bAttacking;

	//Combat Attack Range Sphere Comp
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> CombatRangeSphere;

	//Collision Box for Left Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> LeftWeaponCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> RightWeaponCollisionBox;

	//Base Weapon Damage inflicted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket;

	//Hit Anim Montage when hit by bullet
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Animations", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> HitMontage;

	//Montage containing different Attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Animations", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> AttackMontage;

	//Taunt Montage when Target is detected for first time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat - Animations", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> TauntMontage;

	//===========================================================================================

	//Custom AIController
	TObjectPtr<AEnemyController> EnemyController;

	//Section names of Attack Montage Names
	FName AttackSectionNames[4];

	FTimerHandle HeathBarHideTimerHandle;
	FTimerHandle HitReactTimer;
	FTimerHandle DestroyTimerHandle;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int32 DamageToShow, FVector HitLocation, bool bHeadshot);

	UFUNCTION(BlueprintCallable)
	void PlayTheSound(const FString& SoundName, bool bFadeIn, bool bFadeOut, float VolumeMultiplier);

	UFUNCTION(BlueprintCallable)
	void DetectPlayer(AActor* OtherActor);

	UFUNCTION(BlueprintCallable)
	FVector GetEnemyTargetLocation() const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsAttacking() const { return bAttacking; }

	FORCEINLINE bool HasGreetedPlayer() const { return bGreetedPlayer; }
	FORCEINLINE FString GetHeadBone() const { return HeadBone; }
	FORCEINLINE TObjectPtr<UBehaviorTree> GetBehaviorTree() const { return BehaviorTree; }
	FORCEINLINE TObjectPtr<UHealthComponent> GetHealthComponent() const { return HealthComponent; }
};
