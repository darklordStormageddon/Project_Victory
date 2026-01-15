// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Interact/InteracterComponent.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
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

			_collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::OnTriggerEnter);
			_collisionComponent->OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::OnTriggerExit);
		}
	}
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_isDebugDraw)
	{
		DrawDebugSphere(GetWorld(), _collisionComponent->GetComponentLocation(), _collisionRadius, 16, FColor::Yellow, false, DeltaTime * 1.01f);
	}
}

void UInteractableComponent::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!_isInterrupt && _interacter != nullptr)
		return;

	if (OtherActor == nullptr)
		return;

	// 오브젝트에서 UInteracterComponent 컴포넌트 찾기
	UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
	if (_isInterrupt && _interacter != nullptr && _interacter != _foundInteracter)
	{
		_InterruptInteracter = _foundInteracter;
		_interacter->OnInteractable(this, true);
		return;
	}

	_interacter = _foundInteracter;

	if (_interacter == nullptr)
		return;

	_interacter->OnInteractable(this, false);
}

void UInteractableComponent::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
		return;

	// 오브젝트에서 UInteracterComponent 컴포넌트 찾기
	UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
	if (_foundInteracter == nullptr)
		return;

	// 오브젝트의 UInteracterComponent 컴포넌트와 비교
	if (_interacter != _foundInteracter)
		return;

	// UInteracterComponent 컴포넌트와 UInteractableComponent 컴포넌트 연결 해제
	_interacter = nullptr;
	_foundInteracter->OnDisInteractable();

	if (_isInteract)
	{
		ChangeInteractState(false);
	}
}

void UInteractableComponent::InitializeUIInteractable(bool IsDebugDraw, float InteractRadius, E_INTERACT_TYPE InteractType, E_UI_TYPE InteractUIType)
{
	_isDebugDraw = IsDebugDraw;
	_collisionRadius = InteractRadius;
	_interactType = InteractType;
	_interactUIType = InteractUIType;
}

bool UInteractableComponent::TryInteract(bool& OutIsInterupt, bool& OutIsInteractEnter)
{
	OutIsInterupt = false;
	OutIsInteractEnter = false;
	if (_interacter == nullptr)
		return false;

	if (_isInterrupt && _isInteract)
	{
		// TODO : 작업자 집어 던지기 성공 체크
		bool _isInterruptSuccess = false;
		_InterruptInteracter->OnInteractable(this, !_isInterruptSuccess);
		OutIsInterupt = true;
		return true;
	}

	_isInteract = !_isInteract;
	ChangeInteractState(_isInteract);
	OutIsInteractEnter = _isInteract;
	return true;
}

void UInteractableComponent::ChangeInteractState(bool IsInteract)
{
	_isInteract = IsInteract;
	if (_isInteract)
	{
		if (_interactUIType != E_UI_TYPE::NONE)
		{
			_uiManager->OpenUI(_interactUIType);
		}
		OnInteractEnterAction.Broadcast();
	}
	else
	{
		OnInteractExitAction.Broadcast();
		if (_interactUIType != E_UI_TYPE::NONE)
		{
			_uiManager->CloseUI(_interactUIType);
		}
	}
}