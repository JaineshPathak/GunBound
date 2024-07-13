// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "Curves/CurveFloat.h"
#include "ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"
#include "GameFramework/RotatingMovementComponent.h"

// Sets default values
AItem::AItem() :
	ItemName(FString("Default")),
	AmmoCount(0),
	ItemRarity(EItemRarity::EIR_Common),
	ItemState(EItemState::EIS_Pickup),

	ItemInterpStartLocation(FVector(0.0f)),
	CameraTargetLocation(FVector(0.0f)),
	bInterping(false),
	ZCurveTime(0.7f),
	ItemInterpX(0.0f),
	ItemInterpY(0.0f),
	ItemInitialYawOffset(0.0f),

	ItemType(EItemType::EIT_Ammo),
	InterpLocIndex(0),
	MaterialIndex(0),
	bCanChangeCustomDepth(true),

	//Dynamic Material Parameters
	PulseCurveTime(5.0f),
	GlowAmount(150.0f),
	FresnelExponent(3.0f),
	FresnelReflectFraction(4.0f),

	SlotIndex(0),
	bCharacterInventoryFull(false),

	bIsInteractable(true)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Item Mesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(ItemMesh);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup Widget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area Sphere"));
	AreaSphere->SetupAttachment(GetRootComponent());

	RotatingComponent = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingComponent"));
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	//Hide PickupWidget
	if(PickupWidget)
		PickupWidget->SetVisibility(false);

	//Based on item rarity
	SetActiveStars();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	//Set Item Properties based on Item State
	SetItemProperties(ItemState);

	//Init Custom Depth (Default: Disabled)
	InitCustomDepth();

	//Starts the Pulse Curve Timer in Loop
	StartPulseTimer();
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
			ShooterCharacter->IncrementOverlappedItemCount(1);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
			ShooterCharacter->IncrementOverlappedItemCount(-1);
	}
}

void AItem::SetActiveStars()
{
	for (int32 i = 0; i <= 5; i++)
	{
		//0th element isn't used
		ActiveStars.Add(false);
	}

	switch (ItemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	case EItemRarity::EIR_MAX:
		break;
	default:
		break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AreaSphere)
		{
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}

		if (CollisionBox)
		{
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}

		if (RotatingComponent)
			RotatingComponent->Activate();
		break;

	case EItemState::EIS_EquipInterping:

		if(PickupWidget)
			PickupWidget->SetVisibility(false);

		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AreaSphere)
		{
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (CollisionBox)
		{
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (RotatingComponent)
			RotatingComponent->Deactivate();
		break;

	case EItemState::EIS_PickedUp:
		if (PickupWidget)
			PickupWidget->SetVisibility(false);

		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AreaSphere)
		{
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (CollisionBox)
		{
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (RotatingComponent)
			RotatingComponent->Deactivate();
		break;
	
	case EItemState::EIS_Equipped:

		if (PickupWidget)
			PickupWidget->SetVisibility(false);

		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AreaSphere)
		{
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (CollisionBox)
		{
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (RotatingComponent)
			RotatingComponent->Deactivate();
		break;

	case EItemState::EIS_Falling:

		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(true);
			ItemMesh->SetVisibility(true);
			ItemMesh->SetEnableGravity(true);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		}

		if (AreaSphere)
		{
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (CollisionBox)
		{
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (RotatingComponent)
			RotatingComponent->Activate();
		break;

	case EItemState::EIS_InActive:
		if (PickupWidget)
			PickupWidget->SetVisibility(false);

		if (ItemMesh)
		{
			ItemMesh->SetSimulatePhysics(false);
			ItemMesh->SetEnableGravity(false);
			ItemMesh->SetVisibility(false);
			ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (AreaSphere)
		{
			AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (CollisionBox)
		{
			CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		if (RotatingComponent)
			RotatingComponent->Deactivate();
		break;
	}
}

void AItem::FinishInterping(AShooterCharacter* Char)
{
	if (Character == nullptr && Char != nullptr)
		Character = Char;

	if (Character)
	{
		Character->IncrementInterpLocItemCount(InterpLocIndex, -1);
		Character->GetPickupItem(this);
	}

	bInterping = false;

	PlayEquipSound();
	SetActorScale3D(FVector(1.0f));
	DisableGlowMaterial();
	
	bCanChangeCustomDepth = true;
	DisableCustomDepth();
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		//Get Elasped time from Timer
		const float ElaspedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);

		//Get Curve val corresponding to time
		const float CurveValue = ItemZCurve->GetFloatValue(ElaspedTime);

		//Get Items Initial location
		FVector ItemLocation = ItemInterpStartLocation;

		//Get location in for Interp Location
		const FVector CameraInterpLocation = GetInterpLocation();

		//Vector from item to camera location
		const FVector ItemToCamera(FVector(0.0f, 0.0f, (CameraInterpLocation - ItemLocation).Z));
		const float DeltaZ = ItemToCamera.Size();

		//Interpolated X and Y
		const FVector CurrentLocation = GetActorLocation();
		const float InterpX = FMath::FInterpTo(CurrentLocation.X, CameraInterpLocation.X, DeltaTime, 30.0f);
		const float InterpY = FMath::FInterpTo(CurrentLocation.Y, CameraInterpLocation.Y, DeltaTime, 30.0f);

		ItemLocation.X = InterpX;
		ItemLocation.Y = InterpY;
		ItemLocation.Z += CurveValue * DeltaZ;

		SetActorLocation(ItemLocation, false, nullptr, ETeleportType::TeleportPhysics);

		const FRotator CameraRotation = Character->GetFollowCamera()->GetComponentRotation();
		FRotator ItemRotation{ 0.0f, CameraRotation.Yaw + ItemInitialYawOffset, 0.0f };

		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float CurveScaleValue = ItemScaleCurve->GetFloatValue(ElaspedTime);
			SetActorScale3D(FVector(CurveScaleValue, CurveScaleValue, CurveScaleValue));
		}
	}
}

FVector AItem::GetInterpLocation()
{
	if (Character == nullptr) return FVector(0.0f);

	FVector InterpingLocation{0.0f, 0.f, 0.f};

	switch (ItemType)
	{
	case EItemType::EIT_Ammo:
		return Character->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
		break;
	
	case EItemType::EIT_Weapon:
		return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		break;
	}

	return InterpingLocation;
}

void AItem::PlayPickupSound(bool bForcePlaySound)
{
	if (Character)
	{
		if (bForcePlaySound)
		{
			if (PickupSound)
				UGameplayStatics::PlaySound2D(this, PickupSound);
		}
		else if (Character->ShouldPlayPickupSound())
		{
			Character->StartPickupSoundTimer();
			if(PickupSound)
				UGameplayStatics::PlaySound2D(this, PickupSound);
		}
	}
}

void AItem::PlayEquipSound(bool bForcedPlaySound)
{
	if (Character)
	{
		if (bForcedPlaySound)
		{
			if (EquippedSound)
				UGameplayStatics::PlaySound2D(this, EquippedSound);
		}
		else if (Character->ShouldPlayEquipSound())
		{
			Character->StartEquipSoundTimer();
			if (EquippedSound)
				UGameplayStatics::PlaySound2D(this, EquippedSound);
		}
	}
}

void AItem::EnableCustomDepth()
{
	if(bCanChangeCustomDepth)
		ItemMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	if (bCanChangeCustomDepth)
		ItemMesh->SetRenderCustomDepth(false);
}

void AItem::InitCustomDepth()
{
	DisableCustomDepth();
}

//Called before the game starts
void AItem::OnConstruction(const FTransform& Transform)
{
	//Load the data in the Item Rarity Table
	//Path to Data Table
	FString RarityTablePath = TEXT("/Script/Engine.DataTable'/Game/_Game/DataTable/ItemRarityTable.ItemRarityTable'");
	UDataTable* RarityTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *RarityTablePath));
	if (RarityTableObject)
	{
		FItemRarityTable* RarityRow = nullptr;
		switch (ItemRarity)
		{
		case EItemRarity::EIR_Damaged:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Damaged"), TEXT(""));
			break;
		case EItemRarity::EIR_Common:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Common"), TEXT(""));
			break;
		case EItemRarity::EIR_Uncommon:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("UnCommon"), TEXT(""));
			break;
		case EItemRarity::EIR_Rare:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Rare"), TEXT(""));
			break;
		case EItemRarity::EIR_Legendary:
			RarityRow = RarityTableObject->FindRow<FItemRarityTable>(FName("Legendary"), TEXT(""));
			break;
		}

		if (RarityRow)
		{
			GlowColor = RarityRow->GlowColor;
			LightColor = RarityRow->LightColor;
			DarkColor = RarityRow->DarkColor;
			NumStars = RarityRow->StarsNum;
			IconBackground = RarityRow->IconBackground;

			if (GetItemMesh())
				GetItemMesh()->SetCustomDepthStencilValue(RarityRow->CustomDepthStencilVal);
		}
	}

	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), GlowColor);

		ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
		EnableGlowMaterial();
	}
}

void AItem::UpdatePulse()
{
	float ElaspedTime{};
	FVector CurveVal{};
	switch (ItemState)
	{
	case EItemState::EIS_Pickup:		
		if (PulseCurve)
		{
			ElaspedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
			CurveVal = PulseCurve->GetVectorValue(ElaspedTime);			
		}
		break;

	case EItemState::EIS_EquipInterping:
		if (InterpPulseCurve)
		{
			ElaspedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
			CurveVal = InterpPulseCurve->GetVectorValue(ElaspedTime);
		}
		break;	
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveVal.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExponent"), CurveVal.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelReflectFraction"), CurveVal.Z * FresnelReflectFraction);
	}
}

void AItem::EnableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0.0f);
	}
}

void AItem::DisableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1.0f);
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);

	//Get Curve Values from Pulse Curve and set Dynamic Material Instance parameters
	UpdatePulse();
}

void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_Pickup)
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(State);
}

void AItem::StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound)
{
	//Store character handle
	Character = Char;

	//Get array index in InterpLocations with lowest item count
	InterpLocIndex = Character->GetInterpLocationIndex();
	Character->IncrementInterpLocItemCount(InterpLocIndex, 1);	

	PlayPickupSound(bForcePlaySound);

	//Store initial location
	ItemInterpStartLocation = GetActorLocation();

	//Interping starts
	bInterping = true;

	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().ClearTimer(PulseTimer);
	GetWorldTimerManager().SetTimer(ItemInterpTimer, FTimerDelegate::CreateLambda([&] { FinishInterping(Char); }), ZCurveTime, false);

	const float CameraRotYaw = Character->GetFollowCamera()->GetComponentRotation().Yaw;
	const float ItemRotYaw = GetActorRotation().Yaw;

	//Yaw Offset between Cam and Item
	ItemInitialYawOffset = ItemRotYaw - CameraRotYaw;

	bCanChangeCustomDepth = false;
}
