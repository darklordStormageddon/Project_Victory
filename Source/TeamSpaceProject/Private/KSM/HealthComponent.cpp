// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/HealthComponent.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	// ...
	CurrentHealth = MaxHealth;
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::TakeDamage(float Amount)
{
	if (Amount <= 0.f)
		return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth);

	if (CurrentHealth <= 0.f)
	{
		OnDeath.Broadcast();
	}
}

