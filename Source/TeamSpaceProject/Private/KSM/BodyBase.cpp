// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/BodyBase.h"
#include "KSM/HealthComponent.h"

// Sets default values
ABodyBase::ABodyBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void ABodyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABodyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABodyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABodyBase::Broadcast_Body_Detachment()
{
	OnDetached.Broadcast();
}

void ABodyBase::Damage_Body(float Damage, FVector Location)
{
	if(!bCanDamage)
		return;
	if (HealthComp)
	{
		Spawn_Location = Location;
		HealthComp->TakeDamage(Damage);
	}
}

void ABodyBase::Set_Spawn_Finished(int index)
{
	Containings[index].bSpawn_Finished = true;
}
