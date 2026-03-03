// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/Player/SpaceStation.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"

// Sets default values
ASpaceRader::ASpaceRader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
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
}

void ASpaceRader::InitializeRader()
{
	FRaderData _newraderData;

	USpaceManager* _outSpaceManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetSpaceManager(this, _outSpaceManager))
		return;

	_newraderData.StandardActor = Cast<AActor>(_outSpaceManager->GetSpaceStation());
	_newraderData.RaderRenderRadius = _outSpaceManager->GetSpaceRadius();

	// 레이더 표시 시작
	StartRenderRader(_newraderData);
}

FString ASpaceRader::GetFilePathName()
{
	return ConstantLibrary::Resource.SpaceObject.SPACE_RADER_FOLDER;
}

FString ASpaceRader::GetFileHeaderName()
{
	return ConstantLibrary::Resource.SpaceObject.SPACE_RADER_HEADER;
}