// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "ShooterGameState.generated.h"

class AShooterCharacter;
class AShooterPlayerController;
class AMonsterSpawnPoint;
class UEnemyManagerSubsystem;

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	EMS_PreStart	UMETA(DisplayName = "Pre Start"),
	EMS_Start		UMETA(DisplayName = "Start"),
	EMS_WavePause	UMETA(DisplayName = "Wave Pause"),
	EMS_End			UMETA(DisplayName = "End"),

	EMS_MAX			UMETA(DisplayName = "Default MAX")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurvivalEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalEndEvent, bool, bVictory);

UCLASS()
class STEPHEN_TP_SHOOTER_API AShooterGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	AShooterGameState();

	virtual void HandleBeginPlay() override;

private:
	void StartSurvivalMatch();
	void EndSurvivalMatch(bool bVictory);

	void StartWave();
	void EndWave();

	void StartWavePause();
	void EndWavePause();

	AMonsterSpawnPoint* GetValidMonsterSpawnPoint();
	void SpawnMonster(AMonsterSpawnPoint* SpawnPoint);

	UFUNCTION()
	void OnEnemySpawn(AEnemy* Enemy);
	
	UFUNCTION()
	void OnEnemyDeath(AEnemy* Enemy);

	UFUNCTION()
	void OnPlayerDeath(AActor* KillerActor, AController* KillerController);

public:
	//Delegate for match pre start (hides the main HUD, runs a countdown, etc)
	UPROPERTY(BlueprintAssignable, Category = "Match Delegates", meta = (AllowPrivateAccess = "true"))
	FOnSurvivalEvent OnSurvivalPreStartDelegate;

	//Delegate for match actual start
	UPROPERTY(BlueprintAssignable, Category = "Match Delegates", meta = (AllowPrivateAccess = "true"))
	FOnSurvivalEvent OnSurvivalStartDelegate;

	//Delegate for match end
	UPROPERTY(BlueprintAssignable, Category = "Match Delegates", meta = (AllowPrivateAccess = "true"))
	FOnSurvivalEndEvent OnSurvivalEndDelegate;

	//Delegate for Wave Pause start (used for playing HUD FadeIn/Out Animations)
	UPROPERTY(BlueprintAssignable, Category = "Match Delegates", meta = (AllowPrivateAccess = "true"))
	FOnSurvivalEvent OnSurvivalWavePauseStartDelegate;

	//Delegate for Wave Pause end (used for playing HUD FadeIn/Out Animations)
	UPROPERTY(BlueprintAssignable, Category = "Match Delegates", meta = (AllowPrivateAccess = "true"))
	FOnSurvivalEvent OnSurvivalWavePauseEndDelegate;

private:
	/*Communicate with Game Mode to get rules of data*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	const AShooterGameMode* ShooterGameMode;

	/*Keep track of main character*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* ShooterCharacter;

	/*Keep track of main character controller*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	AShooterPlayerController* ShooterCharacterController;

	/*Keep track of Enemy Manager Subsystem*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	UEnemyManagerSubsystem* EnemyManager;

	/*Current State of Survival Game mode*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	EMatchState MatchState;

	/*Current Ongoing Wave*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	int32 WaveCurrent;

	/*Maximum Wave from Game Mode*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	int32 WaveMax;
	
	/*Current Count of Monsters*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	int32 MonstersCountCurrent;

	/*Max Count of Monsters in Current Wave*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	int32 MonstersCountCurrentWave;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	TArray<AActor*> MonsterSpawnPointsList;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Match State", meta = (AllowPrivateAccess = "true"))
	FMonsterWaveData MonsterWaveDataCurrent;

	FTimerHandle MatchPreStartDelayTimerHandle;
	FTimerHandle MatchPreStartTimerHandle;
	FTimerHandle MatchWavePauseTimerHandle;
	
	FTimerHandle MonsterSpawnTimerHandle;
	
	FTimerHandle MonsterDeathTauntDelayTimer;

public:
	AShooterCharacter* GetShooterCharacter() const { return ShooterCharacter; }
	UEnemyManagerSubsystem* GetEnemyManager() const { return EnemyManager; }

	FORCEINLINE int32 GetCurrentWave() const { return WaveCurrent; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetWavesText() const { return FString("Wave: ") + FString::FromInt(WaveCurrent) + FString("/") + FString::FromInt(WaveMax); }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetMonstersCountText() const { return FString("Monsters: ") + FString::FromInt(MonstersCountCurrentWave); }
};
