// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChangedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthDepletedDelegate, AActor*, KillerActor, AController*, KillerController);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STEPHEN_TP_SHOOTER_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedDelegate OnHealthChangedEvent;
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthDepletedDelegate OnHealthDepletedEvent;

private:
	//Is the Character dead?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Health", meta = (AllowPrivateAccess = "true"))
	bool bIsDead;

	//Health of Character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Health", meta = (AllowPrivateAccess = "true"))
	float Health;

	//Max Health of Character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Health", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	//Interpolated Health calculated every tick
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Health", meta = (AllowPrivateAccess = "true"))
	float HealthInterp;

	//Interpolation Speed for Health Bar
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Health", meta = (AllowPrivateAccess = "true"))
	float HealthInterpSpeed;

	//If this Character has Armor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Armor", meta = (AllowPrivateAccess = "true"))
	bool bHasArmor;

	//Armor of Character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Armor", meta = (AllowPrivateAccess = "true"))
	float Armor;

	//Max Armor of Character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Armor", meta = (AllowPrivateAccess = "true"))
	float MaxArmor;

	//Interpolated Health calculated every tick
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Armor", meta = (AllowPrivateAccess = "true"))
	float ArmorInterp;

	//Interpolation Speed for Health Bar
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Armor", meta = (AllowPrivateAccess = "true"))
	float ArmorInterpSpeed;

public:
	UFUNCTION(BlueprintCallable)
	void OnTakeDamage(const float& DamageAmount, AActor* InstigatorActor, AController* InstigatorController);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetHealth() const { return Health; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetHealthPercent() const { return Health / MaxHealth; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetHealthPercentInterp() const { return HealthInterp; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetArmorPercent() const { return Armor / MaxArmor; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetArmorPercentInterp() const { return ArmorInterp; }

	UFUNCTION(BlueprintCallable)
	void AddHealth(const float& HealthAmount);

	UFUNCTION(BlueprintCallable)
	void SetHealth(const float& HealthAmount);

	UFUNCTION(BlueprintCallable)
	void AddArmor(const float& ArmorAmount);
};
