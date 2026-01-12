// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/AS_WingComponent.h"

// Sets default values for this component's properties
UAS_WingComponent::UAS_WingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	WingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingMesh"));
	WingMesh->SetupAttachment(this);
	// ...
}


// Called when the game starts
void UAS_WingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UAS_WingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

