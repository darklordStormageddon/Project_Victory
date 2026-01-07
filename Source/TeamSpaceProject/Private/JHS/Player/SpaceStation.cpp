// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/SpaceStation.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/SpaceObject/SpaceObjectComponent.h"

// Sets default values
ASpaceStation::ASpaceStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	_rootComponent->SetupAttachment(RootComponent);

	_spaceObjectComponent = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));
}

// Called when the game starts or when spawned
void ASpaceStation::BeginPlay()
{
	Super::BeginPlay();
	
	AJHSGameState* _outGameState = nullptr;
	UStaticFunctionLibrary::GetGameState(_outGameState);
	UE_LOG(LogTemp, Warning, TEXT("%f"), _outGameState->GetSpaceShipMaxHP().MaxHP);
}

// Called every frame
void ASpaceStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}