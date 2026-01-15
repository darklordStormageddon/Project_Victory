// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BodyBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBodyDetached);

class UHealthComponent;

UCLASS()
class ABodyBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABodyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<ABodyBase>> Body_Types;

	UPROPERTY(BlueprintAssignable)
	FOnBodyDetached OnDetached;

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	bool bCanDamage;

public:
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void Broadcast_Body_Detachment();


	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health")
	void Damage_Body(float Damage);
};
