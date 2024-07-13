// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundCue.h"
#include "SoundsDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class STEPHEN_TP_SHOOTER_API USoundsDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TMap<FString, TObjectPtr<USoundCue>> AudioMap;
};
