// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PanelBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPanelDetached);

class UHealthComponent;

UCLASS()
class APanelBase : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APanelBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chaos")
	TArray<TSubclassOf<APanelBase>> Panel_Types;

	UPROPERTY(BlueprintAssignable)
	FOnPanelDetached OnDetached;

	UPROPERTY(BlueprintReadWrite, Category = "Health")
	bool bCanDamage;

public:
	UFUNCTION(BlueprintCallable, Category = "Chaos")
	void Broadcast_Panel_Detachment();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Health")
	void Damage_Panel(float Damage);
};
