#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IDamageable.generated.h"

UINTERFACE(MinimalAPI)
class UDamageable : public UInterface
{
	GENERATED_BODY()
};

class STEPHEN_TP_SHOOTER_API IDamageable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ProcessDamageBasic(const float& DamageAmount, AActor* Shooter, AController* ShooterController);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ProcessDamage(const FHitResult& HitResult, const float& DamageAmount, AActor* Shooter, AController* ShooterController);
};