// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
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

		// 월드 스페이스 UI 디버그 드로우
		if (_isWorldSpaceUI)
		{
			AActor* _owner = GetOwner();
			if (_owner != nullptr)
			{
				// Owner의 Transform을 사용하여 RelativeLocation을 월드 좌표로 변환
				FVector _center = _owner->GetTransform().TransformPosition(_worldUIRelativeLocation);

				// 기본 UI 크기 (1920x1080) * Scale
				float _width = 1920.0f * _worldUIScale;
				float _height = 1080.0f * _worldUIScale;

				// Owner의 Transform을 사용하여 방향 벡터 계산
				FVector _rightVector = _owner->GetTransform().TransformVector(FVector(0, 1, 0)) * (_width / 2.0f);
				FVector _upVector = _owner->GetTransform().TransformVector(FVector(0, 0, 1)) * (_height / 2.0f);

				// 직사각형의 4개 코너 계산
				FVector _topLeft = _center - _rightVector + _upVector;
				FVector _topRight = _center + _rightVector + _upVector;
				FVector _bottomLeft = _center - _rightVector - _upVector;
				FVector _bottomRight = _center + _rightVector - _upVector;

				// 노란색 직사각형 그리기
				DrawDebugLine(GetWorld(), _topLeft, _topRight, FColor::Yellow, false, DeltaTime * 1.01f, 0, 2.0f);
				DrawDebugLine(GetWorld(), _topRight, _bottomRight, FColor::Yellow, false, DeltaTime * 1.01f, 0, 2.0f);
				DrawDebugLine(GetWorld(), _bottomRight, _bottomLeft, FColor::Yellow, false, DeltaTime * 1.01f, 0, 2.0f);
				DrawDebugLine(GetWorld(), _bottomLeft, _topLeft, FColor::Yellow, false, DeltaTime * 1.01f, 0, 2.0f);
			}
		}
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

	// 끌어내리기
	if (_isInterrupt && _interacter != nullptr && _interacter != _foundInteracter)
	{
		_InterruptInteracter = _foundInteracter;
		_interacter->OnInteractable(this, E_INTERACT_TYPE::DumpThrow);
		return;
	}

	_interacter = _foundInteracter;

	if (_interacter == nullptr)
		return;

	if (_isWorldSpaceUI)
	{
		_interacter->OnInteractable(this, E_INTERACT_TYPE::Handle);
	}
	else
	{
		_interacter->OnInteractable(this, E_INTERACT_TYPE::Seat);
	}
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
	_interacter->OnDisInteractable();
	_interacter = nullptr;

	if (_isWorldSpaceUI)
		return;

	if (_isInteract)
	{
		AActor* _caller = _foundInteracter != nullptr ? _foundInteracter->GetOwner() : nullptr;
		ChangeInteractState(false, _caller);
	}
}

void UInteractableComponent::InitializeUIInteractable(bool IsDebugDraw, float InteractRadius, E_INTERACT_TYPE InteractType, E_UI_TYPE InteractUIType, bool IsWorldSpaceUI, FVector WorldUIRelativeLocation, float WorldUIScale)
{
	_isDebugDraw = IsDebugDraw;
	_collisionRadius = InteractRadius;
	_interactType = InteractType;
	_interactUIType = InteractUIType;
	_isWorldSpaceUI = IsWorldSpaceUI;
	_worldUIRelativeLocation = WorldUIRelativeLocation;
	_worldUIScale = WorldUIScale;
}

bool UInteractableComponent::TryInteract(AActor* Caller, bool& OutIsInterupt, bool& IsCloseUI)
{
	OutIsInterupt = false;
	IsCloseUI = false;
	if (_interacter == nullptr)
		return false;

	//if (_isInterrupt && _isInteract)
	//{
	//	// TODO : 작업자 집어 던지기 성공 체크
	//	bool _isInterruptSuccess = false;
	//	_InterruptInteracter->OnInteractable(this, E_INTERACT_TYPE::Seat);
	//	_OutIsInterupt = true;
	//	return true;
	//}

	_isInteract = !_isInteract;
	ChangeInteractState(_isInteract, Caller);

	IsCloseUI = _isInteract;
	if (IsCloseUI && _isWorldSpaceUI)
	{
		IsCloseUI = false;
	}
	
	return true;
}

void UInteractableComponent::ChangeInteractState(bool IsInteract, AActor* Caller)
{
	_isInteract = IsInteract;
	if (_isInteract)
	{
		UUIBase* _openedUI = nullptr;
		if (_interactUIType != E_UI_TYPE::NONE)
		{
			if (_isWorldSpaceUI)
			{
				// 월드 공간 UI
				AActor* _owner = GetOwner();
				if (_owner != nullptr)
				{
					_openedUI = _uiManager->OpenUIInWorld(_interactUIType, _owner, _worldUIRelativeLocation, _worldUIScale);
				}
			}
			else
			{
				_openedUI = _uiManager->OpenUI(_interactUIType);
			}
		}
		OnInteractEnterAction.Broadcast(Caller, _openedUI);
	}
	else
	{
		UUIBase* _closedUI = nullptr;
		if (_interactUIType != E_UI_TYPE::NONE)
		{
			_closedUI = _uiManager->CloseUI(_interactUIType);
		}
		OnInteractExitAction.Broadcast(Caller, _closedUI);
	}
}