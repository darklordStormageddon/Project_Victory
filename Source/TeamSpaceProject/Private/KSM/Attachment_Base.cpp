// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/Attachment_Base.h"
#include "KSM/HealthComponent.h"

// Sets default values
AAttachment_Base::AAttachment_Base()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void AAttachment_Base::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAttachment_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAttachment_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAttachment_Base::Broadcast_Attachment_Detachment()
{
	OnDetached.Broadcast();
}

void AAttachment_Base::Damage_Attachment(float Damage, FVector Location)
{
	if(!bCanDamage)
		return;

	if (HealthComp)
	{
		Spawn_Location = Location;
		HealthComp->TakeDamage(Damage);
	}
}

void AAttachment_Base::Set_Spawn_Finished(int index)
{
	Containings[index].bSpawn_Finished = true;
}

