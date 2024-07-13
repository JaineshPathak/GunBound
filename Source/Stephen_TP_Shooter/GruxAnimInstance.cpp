#include "GruxAnimInstance.h"
#include "Enemy.h"

#include "GameFramework/CharacterMovementComponent.h"

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (Enemy == nullptr)
		Enemy = Cast<AEnemy>(TryGetPawnOwner());

	if (Enemy)
	{
		FVector Velocity = Enemy->GetVelocity();
		Velocity.Z = 0.0f;

		Speed = Velocity.Size();
		bGreetedPlayer = Enemy->HasGreetedPlayer();
		bAttacking = Enemy->IsAttacking();
		bIsInAir = Enemy->GetCharacterMovement()->IsFalling();
		TargetLocation = Enemy->GetTargetLocation();
	}
}
