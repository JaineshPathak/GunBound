// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnemyManagerSubsystem.generated.h"

class AEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemySpawnEvent, AEnemy*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeathEvent, AEnemy*, Enemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDamageEvent, AEnemy*, Enemy);

UCLASS()
class STEPHEN_TP_SHOOTER_API UEnemyManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FOnEnemySpawnEvent OnEnemySpawnDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FOnEnemyDeathEvent OnEnemyDeathDelegate;

	UPROPERTY(BlueprintAssignable, Category = "Delegates", meta = (AllowPrivateAccess = "true"))
	FOnEnemyDamageEvent OnEnemyHitDelegate;
};