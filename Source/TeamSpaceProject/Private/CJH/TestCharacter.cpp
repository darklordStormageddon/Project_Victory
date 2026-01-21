// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/TestCharacter.h"

// Sets default values
ATestCharacter::ATestCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AsteroidComponent = CreateDefaultSubobject<UAsteroidComponent>(TEXT("AsteroidComponent"));
}

// Called when the game starts or when spawned
void ATestCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->SpawnActor<AActor>(TestSpaceShip, GetActorTransform());
}

// Called every frame
void ATestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void ATestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
