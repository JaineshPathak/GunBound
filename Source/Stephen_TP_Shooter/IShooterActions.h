#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IShooterActions.generated.h"

UINTERFACE(MinimalAPI)
class UShooterActions : public UInterface
{
	GENERATED_BODY()
};

class STEPHEN_TP_SHOOTER_API IShooterActions
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HasDied();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddScore(const float& ScoreAmount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetShooterLocation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ProcessPowerup();
};