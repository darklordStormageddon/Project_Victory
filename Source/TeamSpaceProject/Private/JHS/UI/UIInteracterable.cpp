// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIInteracterable.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIInteracter.h"
#include "JHS/Player/JHSPlayerBase.h"

// Sets default values for this component's properties
UUIInteracterable::UUIInteracterable()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UUIInteracterable::BeginPlay()
{
	Super::BeginPlay();

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_uiManager = _outUIManager;

	AActor* _owner = GetOwner();
	if (_owner != nullptr)
	{
		_collisionComponent = NewObject<USphereComponent>(_owner, USphereComponent::StaticClass(), TEXT("CollisionComponent"));
		if (_collisionComponent != nullptr)
		{
			_collisionComponent->SetSphereRadius(_collisionRadius);
			_collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			_collisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			_collisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			_collisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
			_collisionComponent->SetGenerateOverlapEvents(true);

			// Root Component Attach
			USceneComponent* _rootComponent = _owner->GetRootComponent();
			if (_rootComponent != nullptr)
			{
				_collisionComponent->SetupAttachment(_rootComponent);
			}

			_owner->AddInstanceComponent(_collisionComponent);
			_collisionComponent->RegisterComponent();

			_collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UUIInteracterable::OnTriggerEnter);
			_collisionComponent->OnComponentEndOverlap.AddDynamic(this, &UUIInteracterable::OnTriggerExit);
		}
	}
}


// Called every frame
void UUIInteracterable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_isDebugDraw)
	{
		DrawDebugSphere(GetWorld(), _collisionComponent->GetComponentLocation(), _collisionRadius, 16, FColor::Yellow, false, DeltaTime * 1.01f);
	}
}

void UUIInteracterable::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (_uiInteracter != nullptr)
		return;

	if (OtherActor == nullptr)
		return;

	// 플레이어의 UUIInteracter 컴포넌트 찾기
	UUIInteracter* _foundInteracter = OtherActor->FindComponentByClass<UUIInteracter>();
	_uiInteracter = _foundInteracter;

	if (_uiInteracter == nullptr)
		return;

	_uiInteracter->OnInteractable(this);
}

void UUIInteracterable::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
		return;

	// 플레이어의 UUIInteracter 컴포넌트 찾기
	UUIInteracter* _foundInteracter = OtherActor->FindComponentByClass<UUIInteracter>();
	if (_foundInteracter == nullptr)
		return;

	// 현재 등록된 UUIInteracter와 같은지 확인
	if (_uiInteracter != _foundInteracter)
		return;

	// UUIInteracter에서 이 UUIInteracterable 제거
	_uiInteracter = nullptr;
	_foundInteracter->OnDisInteractable();
	if (_isInteract)
	{
		_isInteract = false;
		OnInteractExit.Broadcast();
		CloseInteractUI();
	}
}

void UUIInteracterable::InitializeUIInteractable(bool IsDebugDraw, float InteractRadius, E_UI_TYPE InteractUIType)
{
	_isDebugDraw = IsDebugDraw;
	_collisionRadius = InteractRadius;
	_openUIType = InteractUIType;
}

bool UUIInteracterable::TryInteract()
{
	if (_uiInteracter == nullptr)
	{
		OnInteractExit.Broadcast();
		return false;
	}

	_isInteract = !_isInteract;
	if (_isInteract)
	{
		OpenInteractUI();
		OnInteractEnter.Broadcast();
		return true;
	}
	else
	{
		OnInteractExit.Broadcast();
		CloseInteractUI();
		return false;
	}
}

void UUIInteracterable::OpenInteractUI()
{
	_uiManager->OpenUI(_openUIType);
}

void UUIInteracterable::CloseInteractUI()
{
	_uiManager->CloseUI(_openUIType);
}