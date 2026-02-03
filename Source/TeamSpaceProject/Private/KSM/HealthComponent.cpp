// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/HealthComponent.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"

#include "CJH/TestSpaceShip.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if(GetOwner()->HasAuthority())
		SetCurrentHP(MaxHealth);
}

void UHealthComponent::SetCurrentHP(float Max_HP)
{
	if (GetOwner()->HasAuthority())
		CurrentHealth = MaxHealth = Max_HP;
}

// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

void UHealthComponent::TakeDamage(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;

	if (Amount <= 0.f)
		return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, MaxHealth);

	OnHealthChanged.Broadcast(CurrentHealth);
	OnDamaged.Broadcast(Amount);

	if (CurrentHealth <= 0.f)
		OnDeath.Broadcast();
}

void UHealthComponent::OnRep_CurrentHealth()
{
	OnHealthChanged.Broadcast(CurrentHealth);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}