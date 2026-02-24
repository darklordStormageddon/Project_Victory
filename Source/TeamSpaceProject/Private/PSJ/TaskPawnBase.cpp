// Fill out your copyright notice in the Description page of Project Settings.


#include "PSJ/TaskPawnBase.h"

// Sets default values
ATaskPawnBase::ATaskPawnBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATaskPawnBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATaskPawnBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATaskPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

