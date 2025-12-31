// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceObjectBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "JHS/GameControl/JHSGameMode.h"

// Sets default values
ASpaceObjectBase::ASpaceObjectBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ��Ʈ��ũ ���� Ȱ��ȭ
	bReplicates = true;
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void ASpaceObjectBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeSpaceObject();
}

// Called every frame
void ASpaceObjectBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateMovement(DeltaTime);
}

void ASpaceObjectBase::InitializeSpaceObject()
{
	USpaceObjectManager* OutSpaceObjectManager = nullptr;
	if (!UStaticFunctionLibrary::GetSpaceObjectManager(OutSpaceObjectManager))
		return;

	_spaceObjectManager = OutSpaceObjectManager;
	Send();
}

void ASpaceObjectBase::UpdateMovement(float DeltaTime)
{
	if (HasAuthority())
	{
		MovementTick(DeltaTime);
		UpdateSpaceObjectData(GetSpaceObjectData());
	}
}

void ASpaceObjectBase::UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData)
{
	if (GetNetMode() == NM_Standalone)
	{
		SetActorLocation(NewSpaceObjectData.Location);
		SetActorRotation(NewSpaceObjectData.Rotator);
		return;
	}

	// Authority 일 때는 서버의 위치와 회전을 업데이트 합니다.
	if (HasAuthority())
	{
		// 서버의 위치와 회전을 모든 클라이언트에 전송합니다.
		Multicast_UpdateSpaceObjectData(NewSpaceObjectData);
	}
	else
	{
		// 클라이언트의 위치와 회전을 서버에 전송합니다.
		Server_UpdateSpaceObjectData(NewSpaceObjectData);
	}
}

void ASpaceObjectBase::Server_UpdateSpaceObjectData_Implementation(FSpaceObjectData NewSpaceObjectData)
{
	// 서버의 위치와 회전을 모든 클라이언트에 전송합니다.
	Multicast_UpdateSpaceObjectData(NewSpaceObjectData);
}

void ASpaceObjectBase::Multicast_UpdateSpaceObjectData_Implementation(FSpaceObjectData NewSpaceObjectData)
{
	SetActorLocation(NewSpaceObjectData.Location);
	SetActorRotation(NewSpaceObjectData.Rotator);
}

void ASpaceObjectBase::Send()
{
	if (!HasAuthority())
		return;

	FSpaceObjectData _newSpaceObjectData = GetSpaceObjectData();
	_spaceObjectManager->UpdateSpaceObject(_newSpaceObjectData);

	GetWorld()->GetTimerManager().SetTimer(
		_updateTimerHandle,
		FTimerDelegate::CreateUObject(this, &ASpaceObjectBase::Send),
		_updateInterval,
		false
	);
}

FSpaceObjectData ASpaceObjectBase::GetSpaceObjectData()
{
	FSpaceObjectData _newSpaceObjectData;

	_newSpaceObjectData.SpaceObjectPtr = this;
	_newSpaceObjectData.SpaceObjectType = _spaceObjctType;
	_newSpaceObjectData.Location = GetActorLocation();
	_newSpaceObjectData.Rotator = GetActorRotation();
	return _newSpaceObjectData;
}