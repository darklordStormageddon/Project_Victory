// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "Satellite_Base.generated.h"

class APanelBase;
class ABodyBase;
class AAttachment_Base;
class AConnector_Base;

USTRUCT(BlueprintType)
struct FElem_Set
{
	GENERATED_BODY()

	// --- ¸â¹ö º¯¼ö ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TSubclassOf<AActor> Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int Num;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	bool bSpawn_Finished;

	FElem_Set()
		: Name(nullptr)
		, Num(0)
		, bSpawn_Finished(false)
	{
	}
};

UCLASS()
class ASatellite_Base : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASatellite_Base();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Chaos")
	void Trigger_Destruction();

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<APanelBase>> PanelTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<ABodyBase>> BodyTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<AAttachment_Base>> Attachment_Types;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<AConnector_Base>> Connector_Types;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* Location_DataTable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild4;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild5;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild6;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild7;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneChild8;

private:
	UPROPERTY(EditAnywhere, Category = "Property")
	float MinSpeed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Property")
	float MaxSpeed = 0.0f;

};
