// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/ASManager.h"

#include "CJH/Stability/ASCore.h"
#include "CJH/Stability/ASBody.h"

#include "Kismet/GameplayStatics.h"

// Called when the game starts or when spawned
void AASManager::BeginPlay()
{
	Super::BeginPlay();

	// 폐기물 위성 코어 스폰 함수 호출
	Artifical_Satellite_Core_Spawn();
}

void AASManager::Artifical_Satellite_Core_Spawn()
{
	// 폐기물 스폰 개수 랜덤 결정
	int Spawn_Num = FMath::RandRange(min_Spawn, max_Spawn);

	AActor* Center_Actor = UGameplayStatics::GetActorOfClass(GetWorld(), Center);
	if (!Center_Actor)
		return;

	FVector CenterLocation = Center_Actor->GetActorLocation();

	for (int i = 0; i < Spawn_Num; i++)
	{
		// 스폰 위치와 회전 랜덤 결정
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

		// 폐기물 위성 코어 스폰
		GetWorld()->SpawnActor<AActor>(Core, Spawn_Location, Spawn_Rotation);
	}
}

// 폐기물 위성 본체 스폰 함수
AASBody* AASManager::Artifical_Satellite_Body_Spawn(
	FVector Spawn_Location,
	FRotator Spawn_Rotation,
	int Value)
{
	// 범위 검사 추가
	if (Value < 0 || Value >= Bodies.Num())
		return nullptr;

	// 본체 스폰
	if (Bodies[Value])
		return GetWorld()->SpawnActor<AASBody>(Bodies[Value], Spawn_Location, Spawn_Rotation);
	else
		return nullptr;
}

// 폐기물 위성 날개 스폰 함수
AASWing* AASManager::Artifical_Satellite_Wing_Spawn(
	FVector Spawn_Location,
	FRotator Spawn_Rotation,
	float RestNum,
	int Value,
	bool Direction)
{
	if (RestNum > 0)// 남은 날개가 있을 때만 스폰
	{
		// 다음 날개 스폰
		AASWing* NextWing = GetWorld()->SpawnActor<AASWing>(Wings[Value], Spawn_Location, Spawn_Rotation);

		// 남은 날개 수 감소 및 날개 번호 전달하기
		NextWing->RestWing = RestNum - 1;
		NextWing->Numbering = Value;

		// 날개 방향 설정
		if(Direction)
			NextWing->Direction = true;
		else
			NextWing->Direction = false;

		return NextWing;
	}
	else
		return nullptr;
}

// 랜덤으로 날개 개수 결정 함수
float AASManager::CorrectWingNum() 
{ 
	return FMath::RandRange(min_Wing, max_Wing); 
}

// 본체 종류 배열의 크기 반환 함수
int AASManager::GetBodiesNum() 
{ 
	return Bodies.Num();
}

// 날개 종류 배열의 크기 반환 함수
int AASManager::GetWingsNum()
{ 
	return Wings.Num(); 
}

// 첫번째 날개 부착 거리 범위 반환 함수
float AASManager::GetRandomFirstDist(bool IsMin)
{
	return IsMin ? MinRandomFirstDist : MaxRandomFirstDist;
}