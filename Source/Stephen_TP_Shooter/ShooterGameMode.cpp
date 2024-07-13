// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"

#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"
#include "ShooterGameState.h"

AShooterGameMode::AShooterGameMode() : 
	GameStartCountDown(3),

	WavesMax(10),
	WavesPauseCountDown(10),

	MonstersSpawnDelay(3),
	MonstersCountMax(15)
{
	DefaultPawnClass = AShooterCharacter::StaticClass();
	PlayerControllerClass = AShooterPlayerController::StaticClass();
	GameStateClass = AShooterGameState::StaticClass();
}