// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

class USphereComponent;

UCLASS()
class STEPHEN_TP_SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
	
public:
	AAmmo();

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type Reason) override;

	virtual void Tick(float DeltaTime) override;

protected:
	//Override of SetItemProperties with AmmoMesh
	virtual void SetItemProperties(EItemState State) override;

	//Called when overlapping area sphere
	UFUNCTION()
	void OnAmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	//Mesh for ammo pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	//Type of Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	//Type of Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	float RespawnTime;

	//Pickup State of Ammo (used for Respawning purpose)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	EPickupState PickupState;

	//Icon Texture of Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;

	//Overlap sphere when running over ammo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ammo", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AmmoCollisionSphere;
	
	FTimerHandle RespawnTimerHandle;

public:
	FORCEINLINE void SetPickupState(EPickupState NewPickupState);

	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	virtual void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false) override;

	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;
};
