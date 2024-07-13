// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class AEnemy;

USTRUCT(BlueprintType)
struct FMonsterWaveData
{
	GENERATED_BODY()

	//Maximum amount of Monsters in current wave to prevent overpopulation
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 MaxMonsters;

	//Total count of monsters before the next wave. Decrements after every kill in Game State
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	uint8 MonstersCount;

	//Different Types of Enemy classes
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<AEnemy>> EnemyTypes;

	FMonsterWaveData() :
		MaxMonsters(15),
		MonstersCount(10)
	{}
};

UCLASS()
class STEPHEN_TP_SHOOTER_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AShooterGameMode();
	
private:
	/* Count down at the fresh start of game */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 GameStartCountDown;
	
	/* Max Amount of Waves */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 WavesMax;

	/* Timer between End of current wave and next wave */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 WavesPauseCountDown;

	/* Delay before spawn new fresh monsters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 MonstersSpawnDelay;

	/* Max Count of Monsters spawned in Level to prevent over monster spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	uint8 MonstersCountMax;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FMonsterWaveData> MonsterWavesData;

public:
	FORCEINLINE uint8 GetGameStartCountDown() const { return GameStartCountDown; }
	
	FORCEINLINE uint8 GetWavesMax() const { return WavesMax; }
	FORCEINLINE uint8 GetWavePauseCountDown() const { return WavesPauseCountDown; }

	FORCEINLINE uint8 GetMonstersSpawnDelay() const { return MonstersSpawnDelay; }
	FORCEINLINE uint8 GetMontersCountMax() const { return MonstersCountMax; }

	FORCEINLINE FMonsterWaveData GetMonsterWaveDataFromIndex(int32 Index) const { return MonsterWavesData[Index]; }
};
