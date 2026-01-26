// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"

// Sets default values for this component's properties
UCollectStateGroup::UCollectStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCollectStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCollectStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCollectStateGroup::InitializeCollectState(TObjectPtr<AJHSGameState> GameState)
{
	_gameState = GameState;
}

void UCollectStateGroup::UpdateCollectState()
{

}