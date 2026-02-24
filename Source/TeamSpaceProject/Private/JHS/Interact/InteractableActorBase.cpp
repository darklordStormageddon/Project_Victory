// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableActorBase.h"
#include "JHS/Interact/InteractableComponent.h"

// Sets default values
AInteractableActorBase::AInteractableActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_interacterable = CreateDefaultSubobject<UInteractableComponent>(TEXT("Interactable"));
}

// Called when the game starts or when spawned
void AInteractableActorBase::BeginPlay()
{
	Super::BeginPlay();

	if (_interacterable != nullptr)
	{
		// 델리게이트 바인딩
		_interacterable->InitializeUIInteractable(_isDebugDraw, _interactRadius, E_INTERACT_TYPE::Seat, _interatUIType, _isWorldSpaceUI, _worldUIRelativeLocation, _worldUIScale);
		_interacterable->OnInteractEnterAction.AddDynamic(this, &AInteractableActorBase::InteractEnter);
		_interacterable->OnInteractExitAction.AddDynamic(this, &AInteractableActorBase::InteractExit);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AInteractableActorBase: UInteractableComponent component not found"));
	}
}

// Called every frame
void AInteractableActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableActorBase::InteractEnter(int32 CallerPlayerId, UUIBase* OpenedUI)
{
	OnInteractEnter(CallerPlayerId, OpenedUI);
}

void AInteractableActorBase::InteractExit(int32 CallerPlayerId, UUIBase* ClosedUI)
{
	OnInteractExit(CallerPlayerId, ClosedUI);
}
