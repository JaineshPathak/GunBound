#include "AudioDataComponent.h"

#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

UAudioDataComponent::UAudioDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UAudioDataComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAudioDataComponent::SetAudioComponentInstance(TObjectPtr<UAudioComponent> AudioComp)
{
	if (AudioComp == nullptr) return;

	AudioCompInstance = AudioComp;
}

void UAudioDataComponent::PlayTheSound(const FString& SoundName, bool bFadeIn, bool bFadeOut, float VolumeMultiplier)
{
	if (AudioCompInstance == nullptr || !AudioMap.Contains(SoundName) || AudioMap[SoundName] == nullptr) return;

	auto Sound = AudioMap[SoundName];
	AudioCompInstance->SetSound(Sound);
	AudioCompInstance->SetVolumeMultiplier(VolumeMultiplier);

	if (bFadeIn)
		AudioCompInstance->FadeIn(0.5f);
	else if (bFadeOut)
		AudioCompInstance->FadeOut(0.5f, 1.0f);
	else
		AudioCompInstance->Play();
}