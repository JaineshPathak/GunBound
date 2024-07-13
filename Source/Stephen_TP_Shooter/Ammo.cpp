// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "ShooterCharacter.h"

#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AAmmo::AAmmo()
{
	PrimaryActorTick.bCanEverTick = true;

	bIsInteractable = true;

	//Construct the Ammo Mesh and set it as Root
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
	SetRootComponent(AmmoMesh);

	GetCollisionBox()->SetupAttachment(GetRootComponent());
	GetAreaSphere()->SetupAttachment(GetRootComponent());
	GetPickupWidget()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.0f);
}

void AAmmo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

void AAmmo::BeginPlay()
{
	Super::BeginPlay();

	bIsInteractable = true;

	if(AmmoCollisionSphere)
		AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::OnAmmoSphereOverlap);
}

void AAmmo::EndPlay(EEndPlayReason::Type Reason)
{
	switch (Reason)
	{
	case EEndPlayReason::Destroyed:
	case EEndPlayReason::LevelTransition:
	case EEndPlayReason::EndPlayInEditor:
	case EEndPlayReason::RemovedFromWorld:
	case EEndPlayReason::Quit:
		if (GetWorldTimerManager().IsTimerActive(RespawnTimerHandle))
			GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
		break;
	default:
		if (GetWorldTimerManager().IsTimerActive(RespawnTimerHandle))
			GetWorldTimerManager().ClearTimer(RespawnTimerHandle);
		break;
	}
}

void AAmmo::SetItemProperties(EItemState State)
{
	Super::SetItemProperties(State);

	switch (State)
	{
	case EItemState::EIS_Pickup:

		if (AmmoMesh)
		{
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		break;

	case EItemState::EIS_EquipInterping:

		if (AmmoMesh)
		{
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		break;

	case EItemState::EIS_PickedUp:
		break;

	case EItemState::EIS_Equipped:

		if (AmmoMesh)
		{
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		break;

	case EItemState::EIS_Falling:

		if (AmmoMesh)
		{
			AmmoMesh->SetSimulatePhysics(true);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetEnableGravity(true);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		}

		break;
	}
}

void AAmmo::OnAmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto ShootingCharacter = Cast<AShooterCharacter>(OtherActor))
		{
			StartItemCurve(ShootingCharacter);

			SetItemState(EItemState::EIS_InActive);
			SetPickupState(EPickupState::EIT_Respawning);
		}
	}
}

void AAmmo::SetPickupState(EPickupState NewPickupState)
{
	PickupState = NewPickupState;

	switch (PickupState)
	{
	case EPickupState::EIT_Active:
		if (AmmoMesh)
		{
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			AmmoMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		}

		if (AmmoCollisionSphere)
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		break;

	case EPickupState::EIT_Respawning:
		if (AmmoMesh)
		{
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(false);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AmmoMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}

		if (AmmoCollisionSphere)
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		GetWorldTimerManager().SetTimer(RespawnTimerHandle, FTimerDelegate::CreateLambda([&] 
			{
				SetItemState(EItemState::EIS_Pickup);
				SetPickupState(EPickupState::EIT_Active); 
			}), RespawnTime, false);
		
		break;
	}
}

void AAmmo::StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound)
{
	Character = Char;

	FinishInterping(Char);

	SetItemState(EItemState::EIS_InActive);
	SetPickupState(EPickupState::EIT_Respawning);
}

void AAmmo::EnableCustomDepth()
{
	if (AmmoMesh == nullptr) return;

	AmmoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	if (AmmoMesh == nullptr) return;

	AmmoMesh->SetRenderCustomDepth(false);
}
