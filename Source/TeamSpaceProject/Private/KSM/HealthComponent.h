// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class USpaceShipStateGroup;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamaged, float, Damage);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CurrentHealth();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(	EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float CurrentHealth;

	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDamaged OnDamaged;

	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float Amount);

	void SetCurrentHP(float Max_HP);

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps
	) const override;
};
