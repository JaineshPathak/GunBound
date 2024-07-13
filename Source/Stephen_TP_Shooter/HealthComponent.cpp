// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "ShooterCharacter.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent() :
	bIsDead(false),
	
	Health(100.0f),
	MaxHealth(100.0f),
	HealthInterp(1.0f),
	HealthInterpSpeed(0.15f),
	
	Armor(500.0f),
	MaxArmor(500.0f),
	ArmorInterp(1.0f),
	ArmorInterpSpeed(0.15f)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	const float HealthCurrent = Health / MaxHealth;
	HealthInterp = FMath::FInterpTo(HealthInterp, HealthCurrent, DeltaTime, HealthInterpSpeed);

	if (bHasArmor)
	{
		const float ArmorCurrent = Armor / MaxArmor;
		ArmorInterp = FMath::FInterpTo(ArmorInterp, ArmorCurrent, DeltaTime, ArmorInterpSpeed);
	}
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	bIsDead = false;

	Health = MaxHealth;
	Armor = MaxArmor;
}

void UHealthComponent::OnTakeDamage(const float& DamageAmount, AActor* InstigatorActor, AController* InstigatorController)
{
	if (bIsDead) return;

	if (bHasArmor)
	{
		float ArmorEffectiveness = (Armor / MaxArmor) - 0.001f;
		ArmorEffectiveness = FMath::Clamp(ArmorEffectiveness, 0.0f, ArmorEffectiveness);

		Armor = FMath::Clamp(Armor - DamageAmount, 0.0f, MaxArmor);

		float FinalDamage = DamageAmount - (DamageAmount * ArmorEffectiveness);
		FinalDamage = FMath::Clamp(FinalDamage, 0.0f, FinalDamage);

		Health = FMath::Clamp(Health - FinalDamage, 0.0f, MaxHealth);
	}
	else
		Health = FMath::Clamp(Health - DamageAmount, 0.0f, MaxHealth);

	if (Health <= 0.0f)
	{
		bIsDead = true;

		if(OnHealthDepletedEvent.IsBound())
			OnHealthDepletedEvent.Broadcast(InstigatorActor, InstigatorController);
	}
	else
	{
		if(OnHealthChangedEvent.IsBound())
			OnHealthChangedEvent.Broadcast();
	}
}

void UHealthComponent::AddHealth(const float& HealthAmount)
{
	if (bIsDead) return;

	Health = FMath::Clamp(Health + HealthAmount, 0.0f, MaxHealth);
}

void UHealthComponent::SetHealth(const float& HealthAmount)
{
	if (HealthAmount <= 0)
	{
		Health = MaxHealth;
		return;
	}

	Health = HealthAmount;
}

void UHealthComponent::AddArmor(const float& ArmorAmount)
{
	if (bIsDead) return;

	Armor = FMath::Clamp(Armor + ArmorAmount, 0.0f, MaxArmor);
}