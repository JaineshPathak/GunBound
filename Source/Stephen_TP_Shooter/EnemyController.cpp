// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
#include "Enemy.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Navigation/CrowdFollowingComponent.h"

AEnemyController::AEnemyController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoardComp"));
	check(Blackboard);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	check(BehaviorTreeComponent);
}

AEnemyController::AEnemyController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoardComp"));
	check(Blackboard);

	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComp"));
	check(BehaviorTreeComponent);
}

void AEnemyController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (InPawn == nullptr) return;

	Enemy = Cast<AEnemy>(InPawn);
	if (Enemy && Enemy->GetBehaviorTree())
		Blackboard->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));
}