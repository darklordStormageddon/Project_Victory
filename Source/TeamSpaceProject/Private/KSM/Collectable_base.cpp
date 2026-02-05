// Fill out your copyright notice in the Description page of Project Settings.


#include "KSM/Collectable_base.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/ContainerStateGroup.h"

// Sets default values
ACollectable_base::ACollectable_base()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ore"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetSimulatePhysics(true);

	bReplicates = true;
}

// Called when the game starts or when spawned
void ACollectable_base::BeginPlay()
{
	Super::BeginPlay();

	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;
	
}

// Called every frame
void ACollectable_base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACollectable_base::Add_Elem_To_GameState(E_ELEMENT_TYPE type, int32 amount)
{
	if (!_outGameState)
		return;

	_outGameState->GetContainerStateGroup()->AddElement(type, amount);
}

