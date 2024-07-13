// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"

#include "Enemy.h"
#include "HealthComponent.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"
#include "EnemyManagerSubsystem.h"
#include "MonsterSpawnPoint.h"

#include "Kismet/GameplayStatics.h"

AShooterGameState::AShooterGameState() :
	MatchState(EMatchState::EMS_PreStart),
	WaveCurrent(0),
	WaveMax(0),
	MonstersCountCurrent(0)
{
}

void AShooterGameState::HandleBeginPlay()
{
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMonsterSpawnPoint::StaticClass(), MonsterSpawnPointsList);

	ShooterGameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode());
	ShooterCharacter = Cast<AShooterCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	ShooterCharacter->GetHealthComponent()->OnHealthDepletedEvent.AddDynamic(this, &AShooterGameState::OnPlayerDeath);
	ShooterCharacterController = ShooterCharacter->GetShooterController();

	EnemyManager = GetWorld()->GetSubsystem<UEnemyManagerSubsystem>();
	EnemyManager->OnEnemySpawnDelegate.AddDynamic(this, &AShooterGameState::OnEnemySpawn);
	EnemyManager->OnEnemyDeathDelegate.AddDynamic(this, &AShooterGameState::OnEnemyDeath);

	MatchState = EMatchState::EMS_PreStart;

	WaveCurrent = 0;
	WaveMax = ShooterGameMode->GetWavesMax();

	GetWorldTimerManager().SetTimer(MatchPreStartDelayTimerHandle, FTimerDelegate::CreateLambda([&]
	{
		GetWorldTimerManager().SetTimer(MatchPreStartTimerHandle, this, &AShooterGameState::StartSurvivalMatch, ShooterGameMode->GetGameStartCountDown(), false);

		if (OnSurvivalPreStartDelegate.IsBound())
			OnSurvivalPreStartDelegate.Broadcast();
	}), 1.0f, false);
	
	Super::HandleBeginPlay();
}

void AShooterGameState::StartSurvivalMatch()
{
	StartWave();

	if(OnSurvivalStartDelegate.IsBound())
		OnSurvivalStartDelegate.Broadcast();
	
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString("START SURVIVAL!"));
}

void AShooterGameState::EndSurvivalMatch(bool bVictory)
{
	MatchState = EMatchState::EMS_End;

	if (GetWorldTimerManager().IsTimerActive(MonsterSpawnTimerHandle))
		GetWorldTimerManager().ClearTimer(MonsterSpawnTimerHandle);

	if(OnSurvivalEndDelegate.IsBound())
		OnSurvivalEndDelegate.Broadcast(bVictory);
	
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString("END SURVIVAL!"));
}

void AShooterGameState::StartWave()
{
	MatchState = EMatchState::EMS_Start;

	WaveCurrent++;
	MonsterWaveDataCurrent = ShooterGameMode->GetMonsterWaveDataFromIndex(WaveCurrent - 1);
	MonstersCountCurrentWave = MonsterWaveDataCurrent.MonstersCount;

	GetWorldTimerManager().SetTimer(MonsterSpawnTimerHandle, FTimerDelegate::CreateLambda([&]
		{
			SpawnMonster(GetValidMonsterSpawnPoint());
		}), ShooterGameMode->GetMonstersSpawnDelay(), true);
}

void AShooterGameState::EndWave()
{
	MatchState = EMatchState::EMS_WavePause;

	if (GetWorldTimerManager().IsTimerActive(MonsterSpawnTimerHandle))
		GetWorldTimerManager().ClearTimer(MonsterSpawnTimerHandle);

	StartWavePause();
}

void AShooterGameState::StartWavePause()
{
	GetWorldTimerManager().SetTimer(MatchPreStartDelayTimerHandle, FTimerDelegate::CreateLambda([&]
	{
		if (OnSurvivalWavePauseStartDelegate.IsBound())
			OnSurvivalWavePauseStartDelegate.Broadcast();

		GetWorldTimerManager().SetTimer(MatchWavePauseTimerHandle, this, &AShooterGameState::EndWavePause, ShooterGameMode->GetWavePauseCountDown(), false);
	}), 2.0f, false);
}

void AShooterGameState::EndWavePause()
{
	StartWave();

	if (OnSurvivalWavePauseEndDelegate.IsBound())
		OnSurvivalWavePauseEndDelegate.Broadcast();
}

AMonsterSpawnPoint* AShooterGameState::GetValidMonsterSpawnPoint()
{
	if (MonsterSpawnPointsList.IsEmpty()) return nullptr;

	AMonsterSpawnPoint* SpawnPoint = nullptr;

	int randIndex = FMath::RandRange(0, MonsterSpawnPointsList.Num() - 1);
	if (MonsterSpawnPointsList.IsValidIndex(randIndex))
	{
		if (auto CurrentSpawnPointAtIndex = Cast<AMonsterSpawnPoint>(MonsterSpawnPointsList[randIndex]))
		{
			if (!CurrentSpawnPointAtIndex->IsVisibleToPlayer() && !CurrentSpawnPointAtIndex->IsOccupied())
			{
				SpawnPoint = CurrentSpawnPointAtIndex;
			}
		}
	}

	return SpawnPoint;
}

void AShooterGameState::SpawnMonster(AMonsterSpawnPoint* SpawnPoint)
{
	if (SpawnPoint == nullptr) return;
	if (MonsterWaveDataCurrent.EnemyTypes.IsEmpty()) return;

	TSubclassOf<AEnemy> MonsterType = MonsterWaveDataCurrent.EnemyTypes[FMath::RandRange(0, MonsterWaveDataCurrent.EnemyTypes.Num() - 1)];

	auto NewEnemy = GetWorld()->SpawnActor<AEnemy>(MonsterType);
	if (NewEnemy)
	{
		NewEnemy->SetActorTransform(SpawnPoint->GetActorTransform());
		NewEnemy->DetectPlayer(ShooterCharacter);
	}
}

void AShooterGameState::OnEnemySpawn(AEnemy* Enemy)
{
	if (Enemy == nullptr) return;

	MonstersCountCurrent++;

	if (auto EnemyHealthComp = Enemy->GetHealthComponent())
	{
		if(MatchState == EMatchState::EMS_Start)
			EnemyHealthComp->SetHealth(EnemyHealthComp->GetHealth() + (EnemyHealthComp->GetHealth() * 1.1f) * ((float)WaveCurrent * 0.25f));
	}

	if (MonstersCountCurrent >= MonsterWaveDataCurrent.MaxMonsters && 
		MatchState == EMatchState::EMS_Start &&
		GetWorldTimerManager().IsTimerActive(MonsterSpawnTimerHandle))
	{
		GetWorldTimerManager().PauseTimer(MonsterSpawnTimerHandle);
	}
}

void AShooterGameState::OnEnemyDeath(AEnemy* Enemy)
{
	if (Enemy == nullptr) return;

	if (ShooterCharacter)
		GetWorldTimerManager().SetTimer(MonsterDeathTauntDelayTimer, FTimerDelegate::CreateLambda([&] {ShooterCharacter->PlayKillTauntSound(); }), 1.0f, false);

	MonstersCountCurrent = FMath::Clamp(MonstersCountCurrent - 1, 0, MonstersCountCurrent);
	MonstersCountCurrentWave = FMath::Clamp(MonstersCountCurrentWave - 1, 0, MonstersCountCurrentWave);

	if (MonstersCountCurrent > 0 && MonstersCountCurrentWave <= 0 &&
		GetWorldTimerManager().IsTimerActive(MonsterSpawnTimerHandle))
	{
		GetWorldTimerManager().PauseTimer(MonsterSpawnTimerHandle);
	}

	if (MonstersCountCurrent <= 0 && MonstersCountCurrentWave <= 0)
	{
		(WaveCurrent < WaveMax) ? EndWave() : EndSurvivalMatch(true);
	}
	else if(MonstersCountCurrent < MonsterWaveDataCurrent.MaxMonsters && MonstersCountCurrentWave > 0 &&
		MatchState == EMatchState::EMS_Start &&
		GetWorldTimerManager().IsTimerPaused(MonsterSpawnTimerHandle))
	{
		GetWorldTimerManager().UnPauseTimer(MonsterSpawnTimerHandle);
	}
}

void AShooterGameState::OnPlayerDeath(AActor* KillerActor, AController* KillerController)
{
	EndSurvivalMatch(false);
}
