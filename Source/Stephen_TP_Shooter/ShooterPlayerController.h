// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class AShooterCharacter;
class AShooterGameState;

UCLASS()
class STEPHEN_TP_SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AShooterPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	UFUNCTION()
	void OnMatchStart();

	UFUNCTION()
	void OnMatchEnd(bool bVictory);

	void MoveForward(float val);
	void MoveRight(float val);
	void TurnAtRate(float val);
	void LookUpAtRate(float val);
	
	void Turn(float val);
	void LookUp(float val);

	void Jump();
	void StopJumping();

	void InputFireButtonPressed();
	void InputFireButtonReleased();

	void InputAimingButtonPressed();
	void InputAimingButtonReleased();

	void InputCrouchButtonPressed();
	void InputReloadButtonPressed();
	void InputThrowGrenadeButtonPressed();
	void InputSelectButtonPressed();

	void InputOneKeyPressed();
	void InputTwoKeyPressed();
	void InputThreeKeyPressed();
	void InputFourKeyPressed();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* ShooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	AShooterGameState* ShooterGameState;

	//Reference to Pre Start Overlay Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HUDPreStartOverlayClass;

	//Reference to overall Overlay Class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> HUDOverlayClass;
	
	//Var to hold HUD Overlay Widget after creating it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> HUDPreStartOverlay;

	//Var to hold HUD Overlay Widget after creating it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UUserWidget> HUDOverlay;

	//Look left/right in deg/sec
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;

	//Look up/down in deg/sec
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	//Turn Rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;

	//Look up/down when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	//Turn Rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float AimTurnRate;

	//Look up/down when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float AimLookUpRate;

	//Scale factor for mouse look sensitivity Turn rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;

	//Scale factor for mouse look sensitivity Look up rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	//Scale factor for mouse look sensitivity Turn rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;

	//Scale factor for mouse look sensitivity Look up rate when aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	//True when aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

public:
	TObjectPtr<UUserWidget> GetHUDOverlay() const { return HUDOverlay; }

	FORCEINLINE bool IsAiming() const { return bAiming; }
	FORCEINLINE bool IsFireButtonHeld() const { return bFireButtonPressed; }

private:
	bool bFireButtonPressed;
};
