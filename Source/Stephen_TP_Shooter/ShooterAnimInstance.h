// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponType.h"
#include "ShooterAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EOffsetState : uint8
{
	EOS_Aiming UMETA(DisplayName="Aiming"),
	EOS_Hip UMETA(DisplayName = "Hip"),
	EOS_Reloading UMETA(DisplayName = "Reloading"),
	EOS_Throwing UMETA(DisplayName = "Throwing"),
	EOS_PickingUp UMETA(DisplayName = "Picking Up"),
	EOS_InAir UMETA(DisplayName = "In Air"),

	EOS_MAX UMETA(DisplayName = "Default Max"),
};

/**
 * 
 */
UCLASS()
class STEPHEN_TP_SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UShooterAnimInstance();

	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);

	virtual void NativeInitializeAnimation() override;

protected:
	//Handle Turning in place variables
	void TurnInPlace();

	//Lean when rotating and running. Handles calculations
	void Lean(float DeltaTime);

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/* Offset yaw used for Strafing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/* Offset yaw before frame stopped moving */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	//Character Yaw this frame when standing still and not in air (TIP = "Turn In Place")
	float TIPCharacterYaw;

	//Character Yaw previous frame when standing still and not in air
	float TIPCharacterYawLastFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float RootYawOffset;

	//Rotation curve value this frame
	float RotationCurve;

	//Rotation curva value last frame
	float RotationCurveLastFrame;

	//Pitch of Aim Rotation used for Aim Offset
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	float Pitch;

	//True when reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bReloading;

	//True when Throwing Grenade
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bThrowingGrenade;

	//True when Picking Up an Item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	bool bPickingUp;

	//Offset State used to determine which offset to use
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turn In Place", meta = (AllowPrivateAccess = "true"))
	EOffsetState OffsetState;

	//Character Yaw this frame
	FRotator CharacterRotation;
	
	//Character Yaw last frame
	FRotator CharacterRotationLastFrame;

	//Yaw Delta used for Lean in Running blend space
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Lean", meta = (AllowPrivateAccess = "true"))
	float YawDelta;

	//True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crouching", meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	//True when equipping
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipping", meta = (AllowPrivateAccess = "true"))
	bool bEquipping;

	//Change the recoil weight based on turning, aim
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float WeaponRecoilWeight;

	//True when turning in place
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bTurningInPlace;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	EWeaponType EquippedWeaponType;

	//True when not reloading and equipped
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bShouldUseFABRIK;
};
