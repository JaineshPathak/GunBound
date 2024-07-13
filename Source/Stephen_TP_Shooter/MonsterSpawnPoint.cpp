#include "MonsterSpawnPoint.h"

#include "ShooterGameState.h"
#include "ShooterCharacter.h"
#include "InventoryComponent.h"
#include "Weapon.h"

#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"

AMonsterSpawnPoint::AMonsterSpawnPoint()
{
 	PrimaryActorTick.bCanEverTick = false;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	CapsuleComp->SetupAttachment(GetRootComponent());
	CapsuleComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	OverlapSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphereComp"));
	OverlapSphereComp->SetupAttachment(CapsuleComp);
	OverlapSphereComp->SetSphereRadius(200.0f);

	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	ArrowComp->SetupAttachment(CapsuleComp);
	ArrowComp->ArrowLength = 40.0f;
}

void AMonsterSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(GetWorld()));
}

bool AMonsterSpawnPoint::IsOccupied()
{
	TArray<AActor*> OverlapArray;
	GetOverlappingActors(OverlapArray, ACharacter::StaticClass());

	return OverlapArray.Num() > 0;
}

bool AMonsterSpawnPoint::IsVisibleToPlayer()
{
	if (ShooterGameState->GetShooterCharacter() == nullptr) return false;

	auto& ShooterCharacter = *ShooterGameState->GetShooterCharacter();

	FVector StartTrace = GetActorLocation();
	FVector EndTrace = ShooterCharacter.GetActorLocation();
	EndTrace.Z += ShooterCharacter.GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 0.75f;

	FVector EndDir = (EndTrace - GetActorLocation()).GetUnsafeNormal();
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult HitResult;
	bool bBlockHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECollisionChannel::ECC_Pawn, QueryParams);
	
	float Dot = FVector::DotProduct(ShooterCharacter.GetActorForwardVector(), EndDir);
	
	return (bBlockHit && HitResult.GetActor() == ShooterGameState->GetShooterCharacter() && Dot < 0.0f);
}
