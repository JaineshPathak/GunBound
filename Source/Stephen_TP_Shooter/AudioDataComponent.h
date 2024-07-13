// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AudioDataComponent.generated.h"

class UAudioComponent;
class USoundCue;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STEPHEN_TP_SHOOTER_API UAudioDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAudioDataComponent();

protected:
	virtual void BeginPlay() override;

private:
	//Audio map hard references all the audio used by the actor. Provide a unique audio key and it call PlayTheSound() function
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio", meta = (AllowPrivateAccess = "true"))
	TMap<FString, TObjectPtr<USoundCue>> AudioMap;

	//Audio Component Instance
	TObjectPtr<UAudioComponent> AudioCompInstance;

public:
	FORCEINLINE void SetAudioComponentInstance(TObjectPtr<UAudioComponent> AudioComp);

	UFUNCTION(BlueprintCallable)
	void PlayTheSound(const FString& SoundName, bool bFadeIn, bool bFadeOut, float VolumeMultiplier);
};
