// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameStateStructs.generated.h"

// Forward declaration
class AJHSGameState;

UENUM(BlueprintType)
enum class E_PURCHASE_CATEGORY : uint8
{
	SpaceShip = 0 UMETA(DisplayName = "SpaceShip"),
	CollectTool UMETA(DisplayName = "CollectTool"),
	Turret UMETA(DisplayName = "Turret"),
	Ammo UMETA(DisplayName = "Ammo"),

	END UMETA(DisplayName = "END"),
};

USTRUCT(BlueprintType)
struct FMaxCurrentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentValue = 0.0f;
};

USTRUCT(BlueprintType)
struct FPurchaseData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UTexture2D* Image = nullptr;

	UPROPERTY()
	FString Description = "";

	UPROPERTY()
	FMaxCurrentData Level;

	UPROPERTY()
	FMaxCurrentData Value;

	UPROPERTY()
	float IncreasePerValue = 0.0f;

	UPROPERTY()
	int32 PurchaseDollar = 0;

	UPROPERTY()
	float IncreasePerDollar = 0.0f;
};

#pragma region SpaceShip
UENUM(BlueprintType)
enum class E_SPACE_SHIP_DATA_TYPE : uint8
{
	Shield = 0 UMETA(DisplayName = "Shield"),
	HP UMETA(DisplayName = "HP"),
	Fuel UMETA(DisplayName = "Fuel"),
	MaxSpeed UMETA(DisplayName = "MaxSpeed"),
	Radiation UMETA(DisplayName = "Radiation"),

	NONE UMETA(DisplayName = "NONE"),
};

USTRUCT(BlueprintType)
struct FSpaceShipData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_SPACE_SHIP_DATA_TYPE DataType = E_SPACE_SHIP_DATA_TYPE::NONE;

	UPROPERTY()
	UTexture2D* SpaceShipDataImage = nullptr;

	UPROPERTY()
	FPurchaseData Data;
};
#pragma endregion SpaceShip

#pragma region Player
USTRUCT(BlueprintType)
struct FPlayerStateData
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStateData")
	int32 PlayerUID = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStateData")
	int32 PlayerIdx = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerStateData")
	FMaxCurrentData Radiation;
};
#pragma endregion Player

#pragma region Collect
UENUM(BlueprintType)
enum class E_COLLECT_TOOL_TYPE : uint8
{
	Vacuum = 0 UMETA(DisplayName = "Vacuum"),
	Laser UMETA(DisplayName = "Laser"),
	Drill UMETA(DisplayName = "Drill"),

	NONE UMETA(DisplayName = "NONE"),
}; 

USTRUCT(BlueprintType)
struct FCollectToolData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_COLLECT_TOOL_TYPE CollectToolType = E_COLLECT_TOOL_TYPE::NONE;

	UPROPERTY()
	UTexture2D* CollectToolImage = nullptr;

	// 내구도
	UPROPERTY()
	FPurchaseData Durability;

	// 작업 속도
	UPROPERTY()
	FPurchaseData ToolDamage;
};
#pragma endregion Collect

#pragma region Turret
UENUM(BlueprintType)
enum class E_AMMO_TYPE : uint8
{
	Bullet = 0 UMETA(DisplayName = "Bullet"),
	Cannon UMETA(DisplayName = "Cannon"),
	Missile UMETA(DisplayName = "Missile"),

	NONE UMETA(DisplayName = "NONE"),
};

USTRUCT(BlueprintType)
struct FAmmoData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_AMMO_TYPE AmmoType = E_AMMO_TYPE::NONE;

	UPROPERTY()
	UTexture2D* AmmoImage = nullptr;

	// 탄약 가격
	UPROPERTY()
	FPurchaseData Price;

	// 탄약 용량
	UPROPERTY()
	FPurchaseData ReloadCapacity;

	// 탄약 데미지
	UPROPERTY()
	FPurchaseData AmmoDamage;

	// 보유량
	UPROPERTY()
	int32 AmmoStockpile = 0;
};

UENUM(BlueprintType)
enum class E_TURRET_POSITION : uint8
{
	Main = 0 UMETA(DisplayName = "Main"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),

	END UMETA(DisplayName = "END"),
};

UENUM(BlueprintType)
enum class E_TURRET_TYPE : uint8
{
	DualCannon = 0 UMETA(DisplayName = "DualCannon"),
	GT UMETA(DisplayName = "GT"),
	AT_Missile UMETA(DisplayName = "AT_Missile"),
	AT_GT UMETA(DisplayName = "AT_GT"),
	AT_Cannon UMETA(DisplayName = "AT_Cannon"),

	NONE UMETA(DisplayName = "NONE"),
};

USTRUCT(BlueprintType)
struct FTurretData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_TURRET_TYPE TurretType = E_TURRET_TYPE::NONE;

	UPROPERTY()
	E_AMMO_TYPE AmmoType = E_AMMO_TYPE::NONE;

	UPROPERTY()
	UTexture2D* TurretImage = nullptr;

	UPROPERTY()
	FPurchaseData Price;

	UPROPERTY()
	FPurchaseData Mag;

	UPROPERTY()
	FPurchaseData FireInterval;
};
#pragma endregion Turret

#pragma region Container
UENUM(BlueprintType)
enum class E_ELEMENT_TYPE : uint8
{
	// 알루미늄, 철, 티타늄, 니켈, 구리, 실리콘, 금, 리튬, 탄소섬유
	Aluminum = 0 UMETA(DisplayName = "Aluminum"),
	Iron UMETA(DisplayName = "Iron"),
	Titanium UMETA(DisplayName = "Titanium"),
	Nickel UMETA(DisplayName = "Nickel"),
	Copper UMETA(DisplayName = "Copper"),
	Silicon UMETA(DisplayName = "Silicon"),
	Gold UMETA(DisplayName = "Gold"),
	Lithium UMETA(DisplayName = "Lithium"),
	CarbonFiber UMETA(DisplayName = "CarbonFiber"),

	NONE UMETA(DisplayName = "NONE"),
};

USTRUCT(BlueprintType)
struct FElementData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	E_ELEMENT_TYPE ElementType = E_ELEMENT_TYPE::NONE;

	UPROPERTY()
	FString KRName = "";

	UPROPERTY()
	int32 Price = 0;

	UPROPERTY()
	int32 Amount = 0;

	UPROPERTY()
	UTexture2D* ElementImage = nullptr;
};

USTRUCT(BlueprintType)
struct FContainerState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 OwnedDollar = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxElementStockpile = 0;

	UPROPERTY()
	TMap<E_ELEMENT_TYPE, FElementData> ElementDataMap;

	UPROPERTY()
	TMap<E_AMMO_TYPE, FAmmoData> AmmoDataMap;
};
#pragma endregion Container

USTRUCT(BlueprintType)
struct FPurchaseDataFormat
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InitValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncreasePerValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InitDollar = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IncreasePerDollar = 0.0f;
};

USTRUCT(BlueprintType)
struct FTestRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	FTestRow() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPurchaseDataFormat Data;
};