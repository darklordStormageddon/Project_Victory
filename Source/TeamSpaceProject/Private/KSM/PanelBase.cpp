// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/PanelBase.h"
#include "KSM/HealthComponent.h"

// Sets default values
APanelBase::APanelBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void APanelBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APanelBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APanelBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APanelBase::Broadcast_Panel_Detachment()
{
	OnDetached.Broadcast();
}

void APanelBase::Damage_Panel_Implementation(float Damage)
{
	if (!bCanDamage)
		return;
	if (HealthComponent)
	{
		HealthComponent->TakeDamage(Damage);
	}
}
