// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class AEnemy;
class UBehaviorTreeComponent;

UCLASS()
class STEPHEN_TP_SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyController();
	AEnemyController(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	//Behavior Tree Component for Enemy NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	AEnemy* Enemy;
};