// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MonsterSpawnPoint.generated.h"

class AShooterGameState;
class AShooterCharacter;

class UCapsuleComponent;
class UArrowComponent;
class USphereComponent;

UCLASS()
class STEPHEN_TP_SHOOTER_API AMonsterSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMonsterSpawnPoint();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	AShooterGameState* ShooterGameState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCapsuleComponent> CapsuleComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> OverlapSphereComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArrowComponent> ArrowComp;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	bool IsOccupied();

	UFUNCTION(BlueprintCallable)
	bool IsVisibleToPlayer();
};
