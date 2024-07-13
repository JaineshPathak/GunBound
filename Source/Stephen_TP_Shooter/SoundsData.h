#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundCue.h"
#include "SoundsData.generated.h"

USTRUCT(BlueprintType)
struct STEPHEN_TP_SHOOTER_API FSoundsData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<USoundCue> Sound;
};