// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Hologram/Hologram.h"

// Sets default values
AHologram::AHologram()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHologram::BeginPlay()
{
	Super::BeginPlay();

	
}

// Called every frame
void AHologram::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	HologramTimer();
}

void AHologram::HologramTimer()
{
	GetWorld()->GetTimerManager().SetTimer(
		HologramTimerHandle,
		this,
		&AHologram::SetHologram,
		HologramDuration,
		false
	);
}

//위치 정보를 받아와서 홀로그램에 표기하는 함수
void AHologram::SetHologram()
{
	CallActorInfo();

	//위치 정보를 받아오는 것은 아직 미정
	for(auto& elem : TemporaryInfo)
	{
		GetWorld()->SpawnActor<AActor>();//()
				
	}
}

//필드 내 특정 Actor들을 TemporaryInfo 키에 추가하는 함수
//이중 포문 없이
void AHologram::GetAllActors()
{//이중 포문 사용하지 않고 추가하기
	SavedAllObject.Empty();

	if (SavedShipObject.Num() > 0)
		for (TSubclassOf<AActor> ShipActor : SavedShipObject)
			SavedAllObject.Add(UGameplayStatics::GetActorOfClass(GetWorld(), ShipActor));

	if (SavedStationObject.Num() > 0)
		for (TSubclassOf<AActor> StationActor : SavedStationObject)
			SavedAllObject.Add(UGameplayStatics::GetActorOfClass(GetWorld(), StationActor));

	if (SavedAsternoidObject.Num() > 0)
		for(TSubclassOf<AActor> AsternoidActor : SavedAsternoidObject)
			SavedAllObject.Add(UGameplayStatics::GetActorOfClass(GetWorld(), AsternoidActor));
	
	if (SavedWasteObject.Num() > 0)
		for (TSubclassOf<AActor> WasteActor : SavedWasteObject)
			SavedAllObject.Add(UGameplayStatics::GetActorOfClass(GetWorld(), WasteActor));

	if (SavedEnemyObject.Num() > 0)
		for (TSubclassOf<AActor> EnemyActor : SavedEnemyObject)
			SavedAllObject.Add(UGameplayStatics::GetActorOfClass(GetWorld(), EnemyActor));
}
void AHologram::CallActorInfo()
{
	for (auto& elem : TemporaryInfo)
	{
		elem.Value = elem.Key->GetActorLocation();
	}
}
