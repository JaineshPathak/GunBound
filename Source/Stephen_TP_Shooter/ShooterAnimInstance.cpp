// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "InventoryComponent.h"
#include "HealthComponent.h"
#include "Weapon.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
	Speed(0.0f),
	bIsInAir(false),
	bIsAccelerating(false),
	MovementOffsetYaw(0.0f),
	LastMovementOffsetYaw(0.0f),
	bAiming(false),
	TIPCharacterYaw(0.0f),
	TIPCharacterYawLastFrame(0.0f),
	RootYawOffset(0.0f),
	Pitch(0.0f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Aiming),
	CharacterRotation(FRotator(0.0f)),
	CharacterRotationLastFrame(FRotator(0.0f)),
	YawDelta(0.0f),
	bCrouching(false),
	WeaponRecoilWeight(1.0),
	bTurningInPlace(false),
	EquippedWeaponType(EWeaponType::EWT_MAX),
	bShouldUseFABRIK(false)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if(ShooterCharacter == nullptr)
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

	if (ShooterCharacter)
	{
		if (ShooterCharacter->GetHealthComponent() && ShooterCharacter->GetHealthComponent()->IsDead()) return;

		bCrouching = ShooterCharacter->GetCrouchingStatus();
		bReloading = ShooterCharacter->GetCombateState() == ECombatState::ECS_Reloading;
		bEquipping = ShooterCharacter->GetCombateState() == ECombatState::ECS_Equipping;
		bThrowingGrenade = ShooterCharacter->GetCombateState() == ECombatState::ECS_Throwing;
		bPickingUp = ShooterCharacter->GetCombateState() == ECombatState::ECS_PickingUp;
		bShouldUseFABRIK = ShooterCharacter->GetCombateState() == ECombatState::ECS_Unoccupied || ShooterCharacter->GetCombateState() == ECombatState::ECS_FireTimerInProgress;

		//Get lateral speed of character from velocity
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		//Is in Air?
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		//Is Character accel?
		bIsAccelerating = (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f);

		//Strafing Calculation for Blendspace
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

		if(ShooterCharacter->GetVelocity().Size() > 0.0f)
			LastMovementOffsetYaw = MovementOffsetYaw;

		bAiming = ShooterCharacter->GetAimingStatus();		

		if (bReloading)
			OffsetState = EOffsetState::EOS_Reloading;
		else if (bIsInAir)
			OffsetState = EOffsetState::EOS_InAir;
		else if (ShooterCharacter->GetAimingStatus())
			OffsetState = EOffsetState::EOS_Aiming;
		else
			OffsetState = EOffsetState::EOS_Hip;

		/*FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
		FString MovementRotationMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White, RotationMessage);
			GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White, MovementRotationMessage);
		}*/

		//Check if Shooter Character has valid equipped weapon
		if (ShooterCharacter->GetInventoryComponent()->GetEquippedWeapon())
			EquippedWeaponType = ShooterCharacter->GetInventoryComponent()->GetEquippedWeapon()->GetWeaponType();
		
		TurnInPlace();
		Lean(DeltaTime);
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
	if (Speed > 0 || bIsInAir)
	{
		//Don't want Turning Place Anim when moving
		RootYawOffset = 0.0f;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		RotationCurveLastFrame = 0.0f;
		RotationCurve = 0.0f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

		const float TIPYawDelta{ TIPCharacterYaw - TIPCharacterYawLastFrame };
		
		//RootYawOffset updated and clamped to [-180 to 180]
		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		//1.0 if Turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			bTurningInPlace = true;
			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));
			const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

			//If RootYawOffset > 0? We are turning Left
			//If RootYawOffset < 0? We are turning Right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float RootYawOffsetAbs{ FMath::Abs(RootYawOffset) };
			if (RootYawOffsetAbs > 90)
			{
				const float YawExcess{ RootYawOffsetAbs - 90.0f };
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
			bTurningInPlace = false;

		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, -1.0f, FColor::Blue, FString::Printf(TEXT("Character Yaw: %f"), TIPCharacterYaw));
			GEngine->AddOnScreenDebugMessage(2, -1.0f, FColor::Red, FString::Printf(TEXT("Root Yaw Offset: %f"), RootYawOffset));
		}*/
	}

	//Set the Recoil Weight
	if (bTurningInPlace)
	{
		//1.0 = Full Upper Body Aiming
		if (bReloading || bThrowingGrenade || bPickingUp || bEquipping)
			WeaponRecoilWeight = 1.0f;
		else
			WeaponRecoilWeight = 0.0f;
	}
	else //Not turning (Standing)
	{
		if (bCrouching)
		{
			if (bReloading || bThrowingGrenade || bPickingUp || bEquipping)
				WeaponRecoilWeight = 1.0f;
			else
				WeaponRecoilWeight = 0.1f;
		}
		else
		{
			if (bAiming || bReloading || bEquipping || bThrowingGrenade || bPickingUp)
				WeaponRecoilWeight = 1.0f;
			else
				WeaponRecoilWeight = 0.1f;
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };

	const float Target{ (float)Delta.Yaw / DeltaTime };

	const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.0f) };
	YawDelta = FMath::Clamp(Interp, -90.0f, 90.0f);

	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(3, -1.0f, FColor::Green, FString::Printf(TEXT("Lean Yaw Delta: %f"), YawDelta));
		GEngine->AddOnScreenDebugMessage(4, -1.0f, FColor::Green, FString::Printf(TEXT("Delta.Yaw: %f"), Delta.Yaw));
	}*/
}
