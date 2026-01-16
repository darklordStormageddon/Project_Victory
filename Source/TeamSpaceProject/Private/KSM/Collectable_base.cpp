// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/Collectable_base.h"

// Sets default values
ACollectable_base::ACollectable_base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ore"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetSimulatePhysics(true);
}

// Called when the game starts or when spawned
void ACollectable_base::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollectable_base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

