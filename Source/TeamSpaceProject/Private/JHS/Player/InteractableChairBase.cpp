// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/InteractableChairBase.h"
#include "JHS/UI/UIInteracterable.h"

// Sets default values
AInteractableChairBase::AInteractableChairBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_uiInteracterable = CreateDefaultSubobject<UUIInteracterable>(TEXT("UIInteractable"));
}

// Called when the game starts or when spawned
void AInteractableChairBase::BeginPlay()
{
	Super::BeginPlay();

	if (_uiInteracterable != nullptr)
	{
		// 델리게이트 바인딩
		_uiInteracterable->InitializeUIInteractable(_isDebugDraw, _interactRadius, _interatUIType);
		_uiInteracterable->OnInteractEnter.AddDynamic(this, &AInteractableChairBase::InteractEnter);
		_uiInteracterable->OnInteractExit.AddDynamic(this, &AInteractableChairBase::InteractExit);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AInteractableChairBase: UUIInteracterable component not found"));
	}
}

// Called every frame
void AInteractableChairBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableChairBase::InteractEnter()
{
	OnInteractEnter();
}

void AInteractableChairBase::InteractExit()
{
	OnInteractExit();
}