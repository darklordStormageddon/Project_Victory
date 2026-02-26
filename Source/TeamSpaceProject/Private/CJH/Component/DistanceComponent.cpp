// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Component/DistanceComponent.h"

#include "JHS/GameControl/JHSGameMode.h"

#include "JHS/Event/EventManager.h"

#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "JHS/Player/SpaceStation.h"
#include "PSJ/PSJ_Spaceship.h"
// Sets default values for this component's properties
UDistanceComponent::UDistanceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void UDistanceComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerActor = Cast<APSJ_Spaceship>(GetOwner());

	if (UStaticFunctionLibrary::TryGetGameMode(InGameMode) && InGameMode)
	{
		SpaceRadius = InGameMode->GetSpaceManager()->GetSpaceRadius();
		SpaceStation = InGameMode->GetSpaceManager()->GetSpaceStation();
	}

	if (!UStaticFunctionLibrary::TryGetEventManager(EventManager) || !EventManager)
		return;
	
	OnStartStageHandle = EventManager->AddListener<UEventOnStartStage>(
		[this](UEventOnStartStage* Event)
		{
			HandleStartStage(Event);
		}
	);

	OnEndStageHandle = EventManager->AddListener<UEventOnEndStage>(
		[this](UEventOnEndStage* Event)
		{
			HandleEndStage(Event);
		}
	);
}

void UDistanceComponent::HandleStartStage(UEventOnStartStage* Event)
{
	if (!Event)
		return;

	bDamageEnabled = true;
	StartMeasure(OwnerActor, SpaceStation);
}

void UDistanceComponent::HandleEndStage(UEventOnEndStage* Event)
{
	if (!Event)
		return;

	bDamageEnabled = false;
	StopMeasure();
}

void UDistanceComponent::StartMeasure(AActor* FromTarget, AActor* ToTarget)
{
	TargetA = FromTarget;
	TargetB = ToTarget;

	GetWorld()->GetTimerManager().SetTimer(MesureTimerHandle, this, &UDistanceComponent::MeasureDistance, CheckDistanceTime, true);
}

void UDistanceComponent::StopMeasure()
{
	GetWorld()->GetTimerManager().ClearTimer(MesureTimerHandle);	
}	

void UDistanceComponent::MeasureDistance()
{
	if (!bDamageEnabled)
		return;

	if (!IsValid(TargetA) || !IsValid(TargetB))
		return;

	LastDistance = CalculateDistance(TargetA->GetActorLocation(), TargetB->GetActorLocation());

	if (SpaceRadius * SpaceRadius < LastDistance)
		OnDistanceDamaged.Broadcast();
}

double UDistanceComponent::CalculateDistance(const FVector& FromTarget, const FVector& ToTarget)
{
	return FVector::DistSquared(ToTarget,FromTarget);
}

//void UDistanceComponent::OverDistance()
//{
//	OwnerActor->
//
//}
//SpaceShip이 해줘야 하는 일은 구독 될 함수를 구현하기(피 다는 것)