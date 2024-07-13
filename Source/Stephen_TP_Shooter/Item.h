// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

class UBoxComponent;
class URotatingMovementComponent;
class UWidgetComponent;
class USphereComponent;
class UCurveFloat;
class UCurveVector;
class USoundCue;
class UDataTable;
class AShooterCharacter;

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged			UMETA(DisplayName = "Damaged"),
	EIR_Common			UMETA(DisplayName = "Common"),
	EIR_Uncommon		UMETA(DisplayName = "Uncommon"),
	EIR_Rare			UMETA(DisplayName = "Rare"),
	EIR_Legendary		UMETA(DisplayName = "Legendary"),

	EIR_MAX				UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup				UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping		UMETA(DisplayName = "Equip Interping"),
	EIS_PickedUp			UMETA(DisplayName = "PickedUp"),
	EIS_Equipped			UMETA(DisplayName = "Equipped"),
	EIS_Falling				UMETA(DisplayName = "Falling"),
	EIS_InActive			UMETA(DisplayName = "InActive"),
	
	EIS_MAX					UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo				UMETA(DisplayName = "Ammo"),
	EIT_Weapon				UMETA(DisplayName = "Weapon"),

	EIT_MAX					UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EPickupState : uint8
{
	EIT_Active				UMETA(DisplayName = "Active"),
	EIT_Respawning			UMETA(DisplayName = "InActive"),

	EIT_MAX					UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StarsNum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencilVal;

	FItemRarityTable() :
		GlowColor(FColor::White),
		LightColor(FColor::White),
		DarkColor(FColor::Black),
		StarsNum(5),
		IconBackground(nullptr),
		CustomDepthStencilVal(255)
	{}
};

UCLASS()
class STEPHEN_TP_SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned

	//Called when overlapping area sphere
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	//Called when end overlapping area sphere
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Number of stars active based on rarity
	void SetActiveStars();

	//Sets properties of item components based on state
	virtual void SetItemProperties(EItemState State);

	//Handles Item Interpolation
	void ItemInterp(float DeltaTime);

	//Get Interp Location based on Item Type
	FVector GetInterpLocation();	
	
	virtual void InitCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;	

	void UpdatePulse();
	void ResetPulseTimer();
	void StartPulseTimer();

private:
	//Skeletal Mesh for the Item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> ItemMesh;

	//Line Trace collides to show HUD Widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> CollisionBox;

	//Popup Widget for when player looks at item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> PickupWidget;

	//Enables item tracing when close (overlapped)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> AreaSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URotatingMovementComponent> RotatingComponent;

	//Name of item shown in Pickup Widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	//Item Count Ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 AmmoCount;

	//Item Rarity determines number of stars in pickup widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	//State of the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	//The curve asset for item curve
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> ItemZCurve;

	//Starting location when interping begins
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	//Target location in front of camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	//True if item is interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping;

	//Plays when interping starts
	FTimerHandle ItemInterpTimer;

	//Duration of curve and timer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	//X and Y for Item
	float ItemInterpX;
	float ItemInterpY;

	//Interp Initial Yaw Offset between camera yaw and item yaw
	float ItemInitialYawOffset;

	//Curve used to scale item when it goes down right near the end of interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveFloat> ItemScaleCurve;

	//Sound played when item is picked up
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> PickupSound;

	//Sound Played when item is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundCue> EquippedSound;

	//Type of Item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	//Index of Interp Location this item is interping to
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIndex;

	//Index for the material we like to change runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	//Material instance used with dynamic instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInstance> MaterialInstance;

	//Dynamic instance changed at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;

	bool bCanChangeCustomDepth;

	//Curve to drive the dynamic material parameters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveVector> PulseCurve;

	FTimerHandle PulseTimer;

	//Time for PulseTimer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	//X-Axis from PulseCurve
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	//Y-Axis from PulseCurve
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	//Z-Axis from PulseCurve
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCurveVector> InterpPulseCurve;

	//Icon for this item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> IconItem;

	//Ammo Icon for this item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> AmmoIcon;

	//Item Slot Index in Inventory Array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	//True when the Character's Inventory is Full
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;
	
	//Rarity Data Table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data Table", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> ItemRarityDataTable;

	//Color in Glow Material
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	//Color in pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	//Color in pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	//Num of Stars in Pickup Widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 NumStars;

	//Background Icon for Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTexture2D> IconBackground;

protected:
	//Pointer to character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AShooterCharacter> Character;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bIsInteractable;

public:
	void PlayPickupSound(bool bForcePlaySound = false);
	void PlayEquipSound(bool bForcedPlaySound = false);

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();

	//Called from Shooter Character class
	virtual void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false);

	//Called when item interp timer is finished
	virtual void FinishInterping(AShooterCharacter* Char = nullptr);

	void EnableGlowMaterial();
	void DisableGlowMaterial();

	FORCEINLINE bool IsInteractable() const { return bIsInteractable; }

	FORCEINLINE TObjectPtr<UWidgetComponent> GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE TObjectPtr<USphereComponent> GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE TObjectPtr<UBoxComponent> GetCollisionBox() const { return CollisionBox; }

	FORCEINLINE EItemRarity GetItemRarity() const { return ItemRarity; }

	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	void SetItemState(EItemState State);

	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetItemMesh() const { return ItemMesh; }

	FORCEINLINE TObjectPtr<USoundCue> GetPickupSound() const { return PickupSound; }
	FORCEINLINE TObjectPtr<USoundCue> GetEquippedSound() const { return EquippedSound; }

	FORCEINLINE int32 GetItemCount() const { return AmmoCount; }

	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }

	FORCEINLINE void SetCharacter(TObjectPtr<AShooterCharacter> Char) { Character = Char; }

	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }

	FORCEINLINE void SetPickupSound(USoundCue* NewPickupSound) { PickupSound = NewPickupSound; }
	FORCEINLINE void SetEquipSound(USoundCue* NewEquipSound) { EquippedSound = NewEquipSound; }

	FORCEINLINE void SetItemName(FString NewItemName) { ItemName = NewItemName; }

	FORCEINLINE void SetItemIcon(UTexture2D* NewItemIcon) { IconItem = NewItemIcon; }
	FORCEINLINE void SetAmmoIcon(UTexture2D* NewAmmoIcon) { AmmoIcon = NewAmmoIcon; }

	FORCEINLINE TObjectPtr<UMaterialInstance> GetMaterialInstance() const { return MaterialInstance; }
	FORCEINLINE void SetMaterialInstance(TObjectPtr<UMaterialInstance> NewMatInstance) { MaterialInstance = NewMatInstance; }

	FORCEINLINE TObjectPtr<UMaterialInstanceDynamic> GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	FORCEINLINE void SetDynamicMaterialInstance(TObjectPtr<UMaterialInstanceDynamic> NewMatInstance) { DynamicMaterialInstance = NewMatInstance; }

	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }

	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	FORCEINLINE void SetMaterialIndex(int32 MatIndex) { MaterialIndex = MatIndex; }
};
