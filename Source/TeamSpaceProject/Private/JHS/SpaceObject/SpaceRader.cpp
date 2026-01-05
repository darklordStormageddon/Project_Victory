// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// Sets default values
ASpaceRader::ASpaceRader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_rootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	_rootComponent->SetupAttachment(RootComponent);

	_raderCenter = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RaderCenter"));
	_raderCenter->SetupAttachment(_rootComponent);
}

// Called when the game starts or when spawned
void ASpaceRader::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ASpaceRader::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// _spaceRadius 만큼 DebugDrawSphere 그리기
	if (_spaceStation)
	{
		DrawDebugSphere(GetWorld(), _spaceStation->GetActorLocation(), _spaceRadius, 10, FColor::Yellow, false, DeltaTime * 1.01);
	}

	// _raderRadius 만큼 DebugDrawSphere 그리기
	DrawDebugSphere(GetWorld(), _raderCenter->GetComponentLocation(), _raderRadius, 10, FColor::Blue, false, DeltaTime * 1.01);
}

void ASpaceRader::InitializeRader()
{
	AJHSGameMode* OutGameMode = nullptr;
	if (!UStaticFunctionLibrary::GetGameMode(OutGameMode))
		return;

	_spaceStation = Cast<AActor>(OutGameMode->GetSpaceStation());
	_spaceRadius = OutGameMode->GetSpaceRadius();
	_raderRate = _raderRadius / _spaceRadius;

	// 레이더 표시 시작
	RenderSpaceObjectToRader(_spaceStation, _raderCenter->GetComponentLocation(), _spaceRadius, _raderRate);
}

FString ASpaceRader::GetFileHeaderName()
{
	return "BP_RO";
}