// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/TestSpaceShip.h"

// Sets default values
ATestSpaceShip::ATestSpaceShip()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ShieldRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ShieldRoot"));
	ShieldRoot->SetupAttachment(RootComponent);

	ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
	ShieldMesh->SetupAttachment(ShieldRoot);

	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetHiddenInGame(true);
}

// Called when the game starts or when spawned
void ATestSpaceShip::BeginPlay()
{
	Super::BeginPlay();

}

void ATestSpaceShip::OnTakeDamage(float Damage)
{
	//Super::OnTakeDamage(Damage);

	if (!ShieldMesh)
		return;

	ShieldMesh->SetHiddenInGame(false);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ShieldAlphaTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			ShieldAlphaTimerHandle,
			this,
			&ATestSpaceShip::HideShield,
			ShieldDisplayDuration,
			false
		);
	}
}

void ATestSpaceShip::HideShield()
{
	if (ShieldMesh)
		ShieldMesh->SetHiddenInGame(true);
}

// Called every frame
void ATestSpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}