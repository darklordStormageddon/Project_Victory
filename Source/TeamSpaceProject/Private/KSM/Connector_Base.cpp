// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/Connector_Base.h"
#include "KSM/HealthComponent.h"

// Sets default values
AConnector_Base::AConnector_Base()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AConnector_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AConnector_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AConnector_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AConnector_Base::Damage_Connector(float Damage)
{
	if (HealthComp)
	{
		HealthComp->TakeDamage(Damage);
	}
}