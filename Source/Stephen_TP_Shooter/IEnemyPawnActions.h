#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IEnemyPawnActions.generated.h"

UINTERFACE(MinimalAPI)
class UEnemyPawnActions : public UInterface
{
	GENERATED_BODY()
};

class STEPHEN_TP_SHOOTER_API IEnemyPawnActions
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetStunnedStatus(bool IsStunned);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetAttackingStatus(bool IsAttacking);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetCanMoveStatus(bool CanMove);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetInvestigatingStatus(bool Investigate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetWeaponLeftStatus(bool Activate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetWeaponRightStatus(bool Activate);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FName GetAttackSectionName();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PlayAttack(FName MontageSection, float PlayRate = 1.0f);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsTargetDead();
};