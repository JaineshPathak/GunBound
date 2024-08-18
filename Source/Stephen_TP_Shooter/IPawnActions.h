#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPawnActions.generated.h"

UINTERFACE(MinimalAPI)
class UPawnActions : public UInterface
{
	GENERATED_BODY()
};

class STEPHEN_TP_SHOOTER_API IPawnActions
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void JumpToDestination(FVector Destination);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ApplyStun();
};