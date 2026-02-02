// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASCore.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Player/SpaceStation.h"

AASCore::AASCore()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AASCore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(!bChangeDirection)
		DistanceCheck();

	Move(DeltaTime);
}

void AASCore::Move(float DeltaTime)
{
	FVector NewLocation = GetActorLocation() + (Info.Direction * Info.Speed * DeltaTime);
	AddActorWorldOffset(NewLocation - GetActorLocation());
}

void AASCore::DistanceCheck()
{
	//AJHSGameMode* InGameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());
	//if (!InGameMode)
	//	return;

	//ASpaceStation* SpaceStation = InGameMode->GetSpaceStation();
	//if (!SpaceStation)
	//	return;

	////SpaceRadius 범위를 벗어나면 범위 내 특정 장소를 랜덤으로 설정 해서 도달 후 방향 재설정
	//float Distance = FVector::Dist(SpaceStation->GetActorLocation(), GetActorLocation());
	//float Radius = InGameMode->GetSpaceRadius();
	//
	//if (!bChangeDirection && Distance > Radius)
	//{
	//	FTimerHandle TimerHandle;

	//	ReSetVector(SpaceStation, Radius);
	//	bChangeDirection = true;

	//	GetWorld()->GetTimerManager().SetTimer(
	//		TimerHandle,
	//		this,
	//		&AASCore::ResetChangeDirection,
	//		ResetChangeDelay,
	//		false
	//	);
	//}
}

void AASCore::ReSetVector(ASpaceStation* SpaceStation, float Radius)
{
	FVector TargetLocation = FVector(
		FMath::RandRange(SpaceStation->GetActorLocation().X - Radius, SpaceStation->GetActorLocation().X + Radius),
		FMath::RandRange(SpaceStation->GetActorLocation().Y - Radius, SpaceStation->GetActorLocation().Y + Radius),
		FMath::RandRange(SpaceStation->GetActorLocation().Z - Radius, SpaceStation->GetActorLocation().Z + Radius)
	);
	Info.Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
}

void AASCore::ResetChangeDirection()
{
	bChangeDirection = false;
}