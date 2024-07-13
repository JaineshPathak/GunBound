// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "ShooterCharacter.h"
#include "ShooterGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

AShooterPlayerController::AShooterPlayerController() :
	//Base Rates for Turning / Looking Up or Down
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),

	//Turn Rates for Aiming or not Aiming
	HipTurnRate(90.0f),
	HipLookUpRate(90.0f),
	AimTurnRate(20.0f),
	AimLookUpRate(20.0f),

	//Mouse Look Sensitivity Scale factors
	MouseHipTurnRate(1.0f),
	MouseHipLookUpRate(1.0f),
	MouseAimingTurnRate(0.6f),
	MouseAimingLookUpRate(0.6f),

	//Aiming Flag
	bAiming(false),
	bFireButtonPressed(false)
{
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//Check HUD Overlay TSub Class Var
	if (HUDOverlayClass)
	{
		HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayClass);
		if (HUDOverlay)
		{
			HUDOverlay->AddToViewport();
			HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (HUDPreStartOverlayClass)
	{
		HUDPreStartOverlay = CreateWidget<UUserWidget>(this, HUDPreStartOverlayClass);
		if (HUDPreStartOverlay) 
			HUDPreStartOverlay->AddToViewport();
	}

	//Get Reference of Shooter Character
	ShooterCharacter = Cast<AShooterCharacter>(GetPawn());

	//Get Reference of Shooter Game State
	ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(GetWorld()));
	ShooterGameState->OnSurvivalStartDelegate.AddDynamic(this, &AShooterPlayerController::OnMatchStart);
	ShooterGameState->OnSurvivalEndDelegate.AddDynamic(this, &AShooterPlayerController::OnMatchEnd);

	DisableInput(this);
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAxis("MoveForward", this, &AShooterPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AShooterPlayerController::MoveRight);

	InputComponent->BindAxis("TurnRate", this, &AShooterPlayerController::TurnAtRate);
	InputComponent->BindAxis("LookUpRate", this, &AShooterPlayerController::LookUpAtRate);
	InputComponent->BindAxis("Turn", this, &AShooterPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &AShooterPlayerController::LookUp);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AShooterPlayerController::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &AShooterPlayerController::StopJumping);

	InputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterPlayerController::InputFireButtonPressed);
	InputComponent->BindAction("FireButton", IE_Released, this, &AShooterPlayerController::InputFireButtonReleased);

	InputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterPlayerController::InputAimingButtonPressed);
	InputComponent->BindAction("AimingButton", IE_Released, this, &AShooterPlayerController::InputAimingButtonReleased);

	InputComponent->BindAction("CrouchButton", IE_Pressed, this, &AShooterPlayerController::InputCrouchButtonPressed);

	InputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterPlayerController::InputReloadButtonPressed);

	InputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AShooterPlayerController::InputThrowGrenadeButtonPressed);

	InputComponent->BindAction("Select", IE_Pressed, this, &AShooterPlayerController::InputSelectButtonPressed);

	InputComponent->BindAction("1Key", IE_Pressed, this, &AShooterPlayerController::InputOneKeyPressed);
	InputComponent->BindAction("2Key", IE_Pressed, this, &AShooterPlayerController::InputTwoKeyPressed);
	InputComponent->BindAction("3Key", IE_Pressed, this, &AShooterPlayerController::InputThreeKeyPressed);
	InputComponent->BindAction("4Key", IE_Pressed, this, &AShooterPlayerController::InputFourKeyPressed);
}

void AShooterPlayerController::OnMatchStart()
{
	EnableInput(this);

	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString("START CONTROLLER SURVIVAL!"));

	if (HUDPreStartOverlay) HUDPreStartOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	if (HUDOverlay) HUDOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void AShooterPlayerController::OnMatchEnd(bool bVictory)
{
	if (bFireButtonPressed) bFireButtonPressed = false;

	DisableInput(this);

	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, FString("END CONTROLLER SURVIVAL!"));
}

void AShooterPlayerController::MoveForward(float val)
{
	if (ShooterCharacter == nullptr) return;

	if (val != 0.0f)
	{
		const FRotator Rotation{ GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		ShooterCharacter->AddMovementInput(Direction, val);
	}
}

void AShooterPlayerController::MoveRight(float val)
{
	if (ShooterCharacter == nullptr) return;

	if (val != 0.0f)
	{
		const FRotator Rotation{ GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		ShooterCharacter->AddMovementInput(Direction, val);
	}
}

void AShooterPlayerController::TurnAtRate(float val)
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->AddControllerYawInput(val * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterPlayerController::LookUpAtRate(float val)
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->AddControllerPitchInput(val * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterPlayerController::Turn(float val)
{
	if (ShooterCharacter == nullptr) return;

	float TurnScaleFactor{};

	TurnScaleFactor = bAiming ? MouseAimingTurnRate : MouseHipTurnRate;
	ShooterCharacter->AddControllerYawInput(val * TurnScaleFactor);
}

void AShooterPlayerController::LookUp(float val)
{
	if (ShooterCharacter == nullptr) return;

	float LookUpScaleFactor{};

	LookUpScaleFactor = bAiming ? MouseAimingLookUpRate : MouseHipLookUpRate;
	ShooterCharacter->AddControllerPitchInput(val * LookUpScaleFactor);
}

void AShooterPlayerController::Jump()
{
	(ShooterCharacter == nullptr) ? GetCharacter()->Jump() : ShooterCharacter->Jump();
}

void AShooterPlayerController::StopJumping()
{
	(ShooterCharacter == nullptr) ? GetCharacter()->StopJumping() : ShooterCharacter->StopJumping();
}

void AShooterPlayerController::InputFireButtonPressed()
{
	if (ShooterCharacter == nullptr) return;

	bFireButtonPressed = true; 
	ShooterCharacter->FiringPressed();
}

void AShooterPlayerController::InputFireButtonReleased() 
{ 
	if (ShooterCharacter == nullptr) return;

	bFireButtonPressed = false;
	ShooterCharacter->FiringReleased();
}

void AShooterPlayerController::InputAimingButtonPressed()
{
	if (ShooterCharacter == nullptr) return;

	bAiming = true;
	ShooterCharacter->AimingPressed();
}

void AShooterPlayerController::InputAimingButtonReleased()
{
	if (ShooterCharacter == nullptr) return;

	bAiming = false;
	ShooterCharacter->AimingReleased();
}

void AShooterPlayerController::InputCrouchButtonPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->CrouchPressed();
}

void AShooterPlayerController::InputReloadButtonPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->ReloadWeapon();
}

void AShooterPlayerController::InputThrowGrenadeButtonPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->ThrowGrenade();
}

void AShooterPlayerController::InputSelectButtonPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->Interact();
}

void AShooterPlayerController::InputOneKeyPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->SwitchToWeapon(0);
}

void AShooterPlayerController::InputTwoKeyPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->SwitchToWeapon(1);
}

void AShooterPlayerController::InputThreeKeyPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->SwitchToWeapon(2);
}

void AShooterPlayerController::InputFourKeyPressed()
{
	if (ShooterCharacter == nullptr) return;

	ShooterCharacter->SwitchToWeapon(3);
}
