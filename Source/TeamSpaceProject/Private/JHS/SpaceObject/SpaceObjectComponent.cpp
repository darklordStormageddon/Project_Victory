// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/SpaceObject/SpaceObjectComponent.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"

// Sets default values for this component's properties
USpaceObjectComponent::USpaceObjectComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpaceObjectComponent::BeginPlay()
{
	Super::BeginPlay();

	// 붙어있는 Actor를 Owner로 설정
	_owner = GetOwner();
	if (!_owner)
	{
		UE_LOG(LogTemp, Error, TEXT("SpaceObjectComponent: BeginPlay: Owner is nullptr"));
		return;
	}

	InitializeSpaceObject();
}


// Called every frame
void USpaceObjectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void USpaceObjectComponent::InitializeSpaceObject()
{
	USpaceObjectManager* OutSpaceObjectManager = nullptr;
	if (!UStaticFunctionLibrary::GetSpaceObjectManager(OutSpaceObjectManager))
		return;

	_spaceObjectManager = OutSpaceObjectManager;
	Send();
}

void USpaceObjectComponent::Send()
{
	FSpaceObjectData _spaceObjectData = GetSpaceObjectData();
	UpdateSpaceObjectData(_spaceObjectData);

	if (!_owner->HasAuthority())
		return;

	// Manager에 업데이트
	_spaceObjectManager->UpdateSpaceObject(_spaceObjectData);

	GetWorld()->GetTimerManager().SetTimer(
		_updateTimerHandle,
		FTimerDelegate::CreateUObject(this, &USpaceObjectComponent::Send),
		_updateInterval,
		false
	);
}

void USpaceObjectComponent::UpdateSpaceObjectData(FSpaceObjectData NewSpaceObjectData)
{
	if (GetNetMode() == NM_Standalone)
	{
		// Standalone 모드에서는 Manager에만 업데이트 (로컬에서 위치 관리)
		_spaceObjectManager->UpdateSpaceObject(NewSpaceObjectData);
		return;
	}

	// Authority 일 때는 서버의 위치와 회전을 모든 클라이언트에 동기화
	if (_owner->HasAuthority())
	{
		// Manager에 업데이트
		_spaceObjectManager->UpdateSpaceObject(NewSpaceObjectData);
		
		// 모든 클라이언트에 위치와 회전 동기화
		Multicast_UpdateSpaceObjectData(NewSpaceObjectData);
	}
	else
	{
		// 클라이언트의 위치와 회전을 서버에 전송
		Server_UpdateSpaceObjectData(NewSpaceObjectData);
	}
}

void USpaceObjectComponent::Server_UpdateSpaceObjectData_Implementation(FSpaceObjectData NewSpaceObjectData)
{
	// 서버의 위치와 회전을 모든 클라이언트에 동기화
	Multicast_UpdateSpaceObjectData(NewSpaceObjectData);
}

void USpaceObjectComponent::Multicast_UpdateSpaceObjectData_Implementation(FSpaceObjectData NewSpaceObjectData)
{
	// 서버와 클라이언트의 좌표 및 회전 동기화
	if (_owner)
	{
		_owner->SetActorLocation(NewSpaceObjectData.Location);
		_owner->SetActorRotation(NewSpaceObjectData.Rotator);
	}
}

FSpaceObjectData USpaceObjectComponent::GetSpaceObjectData()
{
	FSpaceObjectData _newSpaceObjectData;

	_newSpaceObjectData.SpaceObjectComponent = this;
	_newSpaceObjectData.SpaceObjectType = _spaceObjctType;
	_newSpaceObjectData.Location = _owner->GetActorLocation();
	_newSpaceObjectData.Rotator = _owner->GetActorRotation();
	return _newSpaceObjectData;
}