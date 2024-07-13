// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GetProjectVersion.generated.h"

UCLASS()
class STEPHEN_TP_SHOOTER_API UGetProjectVersion : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintPure, meta=(DisplayName = "Get Project Version", CompactNodeTitle = "GetProjectVersion", Category = "System Information"))
	static FString GetProjectVersion();
};
