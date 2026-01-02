// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASManager.h"

#include "CJH/Stability/ASCore.h"
#include "CJH/Stability/ASBody.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AASManager::AASManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AASManager::BeginPlay()
{
	Super::BeginPlay();

	Artifical_Satellite_Core_Spawn();
}

// Called every frame
void AASManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AASManager::Artifical_Satellite_Core_Spawn()
{
	float Spawn_Num = FMath::RandRange(min_Spawn, max_Spawn);

	AActor* Center_Actor = UGameplayStatics::GetActorOfClass(GetWorld(), Center);
	if (!Center_Actor)
		return;

	FVector CenterLocation = Center_Actor->GetActorLocation();

	for (int i = 0; i < Spawn_Num; i++)
	{
		FVector Spawn_Location = FVector(
			FMath::RandRange(CenterLocation.X - Spawn_Distance, CenterLocation.X + Spawn_Distance),
			FMath::RandRange(CenterLocation.Y - Spawn_Distance, CenterLocation.Y + Spawn_Distance),
			FMath::RandRange(CenterLocation.Z - Spawn_Distance, CenterLocation.Z + Spawn_Distance)
		);

		FRotator Spawn_Rotation = FRotator(
			FMath::RandRange(0, 360),
			FMath::RandRange(0, 360),
			FMath::RandRange(0, 360)
		);

		GetWorld()->SpawnActor<AActor>(Core, Spawn_Location, Spawn_Rotation);
	}
}

AASBody* AASManager::Artifical_Satellite_Body_Spawn(
	FVector Spawn_Location,
	FRotator Spawn_Rotation,
	int Value)
{
	// 범위 검사 추가
	if (Value < 0 || Value >= Bodies.Num())
		return nullptr;

	if (Bodies[Value])
		return GetWorld()->SpawnActor<AASBody>(Bodies[Value], Spawn_Location, Spawn_Rotation);
	else
		return nullptr;
}
AASWing* AASManager::Artifical_Satellite_Wing_Spawn(
	FVector Spawn_Location,
	FRotator Spawn_Rotation,
	float RestNum,
	int Value,
	bool Direction)
{
	if (RestNum > 0)
	{
		AASWing* NextWing = GetWorld()->SpawnActor<AASWing>(Wings[Value], Spawn_Location, Spawn_Rotation);
		NextWing->RestWing = RestNum - 1;
		NextWing->Numbering = Value;

		if(Direction)
			NextWing->Direction = true;
		else
			NextWing->Direction = false;

		return NextWing;
	}
	else
		return nullptr;
}

float AASManager::CorrectWingNum() { return FMath::RandRange(min_Wing, max_Wing); }

int AASManager::GetBodiesNum() { return Bodies.Num(); }
int AASManager::GetWingsNum() { return Wings.Num(); }