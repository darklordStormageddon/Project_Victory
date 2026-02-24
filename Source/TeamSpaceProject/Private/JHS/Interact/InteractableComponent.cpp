// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
#include "JHS/Interact/InteracterComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

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

	if (_isDebugDraw && _collisionComponent != nullptr)
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

bool UInteractableComponent::GetOrCacheLocalPlayerController(AJHSPlayerController*& OutController)
{
	// 캐시가 유효하고 같은 월드이면 재사용 (PIE에서 월드별로 올바른 로컬 PC 사용)
	UWorld* _world = GetWorld();
	if (_world != nullptr && _cachedLocalPlayerController.IsValid() && _cachedLocalPlayerController->IsLocalPlayerController()
		&& _cachedLocalPlayerController->GetWorld() == _world)
	{
		OutController = _cachedLocalPlayerController.Get();
		return true;
	}
	_cachedLocalPlayerController.Reset();

	// 이 컴포넌트가 속한 월드에서 "현재 컨텍스트의 로컬" PlayerController 사용.
	// GetFirstPlayerController()는 월드 내 첫 번째 PC(호스트)만 반환하므로, PIE 리슨 서버 등에서 클라이언트 창에서는 잘못된 PC가 선택됨.
	// 따라서 iterator로 순회하여 IsLocalPlayerController()인 PC를 사용.
	if (_world == nullptr)
		return false;
	APlayerController* _pc = nullptr;
	for (FConstPlayerControllerIterator _it = _world->GetPlayerControllerIterator(); _it; ++_it)
	{
		APlayerController* _candidate = _it->Get();
		if (_candidate != nullptr && _candidate->IsLocalPlayerController())
		{
			_pc = _candidate;
			break;
		}
	}
	if (_pc == nullptr)
		return false;
	AJHSPlayerController* _controller = Cast<AJHSPlayerController>(_pc);
	if (_controller == nullptr)
		return false;

	_cachedLocalPlayerController = _controller;
	OutController = _controller;
	return true;
}

bool UInteractableComponent::FindLocalPlayerControllerByAssignedId(UWorld* World, int32 AssignedPlayerId, AJHSPlayerController*& OutController)
{
	OutController = nullptr;
	if (World == nullptr || AssignedPlayerId < 0)
		return false;
	for (FConstPlayerControllerIterator _it = World->GetPlayerControllerIterator(); _it; ++_it)
	{
		APlayerController* _pc = _it->Get();
		if (_pc == nullptr || !_pc->IsLocalPlayerController())
			continue;
		AJHSPlayerController* _jhsPC = Cast<AJHSPlayerController>(_pc);
		if (_jhsPC == nullptr || _jhsPC->GetAssignedPlayerId() != AssignedPlayerId)
			continue;
		OutController = _jhsPC;
		return true;
	}
	return false;
}

void UInteractableComponent::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
		return;

	APawn* _otherPawn = Cast<APawn>(OtherActor);
	if (_otherPawn == nullptr)
		return;

	UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
	if (_foundInteracter == nullptr)
		return;

	AActor* _owner = GetOwner();
	const bool _bAuthority = _owner != nullptr && _owner->HasAuthority();
	UE_LOG(LogTemp, Log, TEXT("[InteractFlow] OnTriggerEnter - %s overlap with OtherActor=%s"),
		_bAuthority ? TEXT("Server") : TEXT("Client"), *GetNameSafe(OtherActor));

	if (_bAuthority)
	{
		UE_LOG(LogTemp, Log, TEXT("[InteractFlow] OnTriggerEnter - Server path: ExecuteServerTriggerEnter"));
		ExecuteServerTriggerEnter(OtherActor);
		return;
	}

	if (!_otherPawn->IsLocallyControlled())
	{
		UE_LOG(LogTemp, Log, TEXT("[InteractFlow] OnTriggerEnter - Client SKIP (OtherPawn not locally controlled)"));
		return;
	}

	AJHSPlayerController* _clientJHSPC = Cast<AJHSPlayerController>(_otherPawn->GetController());
	const int32 _clientAssignedId = _clientJHSPC != nullptr ? _clientJHSPC->GetAssignedPlayerId() : -1;
	UWorld* _clientWorld = GetWorld();
	const ENetMode _clientNetMode = _clientWorld != nullptr ? _clientWorld->GetNetMode() : NM_Standalone;
	UE_LOG(LogTemp, Log, TEXT("[InteractFlow] OnTriggerEnter - Client path: ServerReportTriggerEnter RPC (로컬 AssignedPlayerId=%d NetMode=%d)"), _clientAssignedId, (int32)_clientNetMode);
	_foundInteracter->ServerReportTriggerEnter(this);
}

void UInteractableComponent::ExecuteTriggerEnterForLocalPlayer(E_INTERACT_TYPE InteractType)
{
	if (!IsValid(this))
		return;
	AJHSPlayerController* _localController = nullptr;
	if (!GetOrCacheLocalPlayerController(_localController))
		return;
	APawn* _localPawn = _localController->GetPawn();
	UInteracterComponent* _foundInteracter = _localPawn != nullptr ? _localPawn->FindComponentByClass<UInteracterComponent>() : nullptr;
	if (_foundInteracter == nullptr)
		return;
	_interacter = _foundInteracter;
	_foundInteracter->OnInteractable(this, InteractType);
}

void UInteractableComponent::ExecuteTriggerExitForLocalPlayer()
{
	if (!IsValid(this))
		return;
	AJHSPlayerController* _localController = nullptr;
	if (!GetOrCacheLocalPlayerController(_localController))
		return;
	APawn* _localPawn = _localController->GetPawn();
	UInteracterComponent* _foundInteracter = _localPawn != nullptr ? _localPawn->FindComponentByClass<UInteracterComponent>() : nullptr;
	if (_foundInteracter == nullptr)
		return;
	_interacter = _foundInteracter;
	_foundInteracter->OnDisInteractable();
	_interacter = nullptr;
	_isInteract = false;

	// 비월드 UI: 트리거 이탈 시 이 클라이언트에서 열린 인터랙트 UI 닫기 (서버의 ChangeInteractState는 호스트만 대상이라 일반 클라이언트 UI는 닫지 못함)
	if (!_isWorldSpaceUI && _interactUIType != E_UI_TYPE::NONE)
	{
		UUIManager* _callerUIManager = _localController->GetUIManager();
		if (_callerUIManager != nullptr)
		{
			UUIBase* _closedUI = _callerUIManager->CloseUI(_interactUIType);
			OnInteractExitAction.Broadcast(_localController->GetAssignedPlayerId(), _closedUI);
		}
	}
}

void UInteractableComponent::ExecuteServerTriggerEnter(AActor* OtherActor)
{
	if (OtherActor == nullptr)
		return;

	if (!_isInterrupt && _interacter != nullptr)
		return;

	UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
	if (_foundInteracter == nullptr)
		return;

	UWorld* _world = GetWorld();
	APawn* _otherPawnForId = Cast<APawn>(OtherActor);
	AJHSPlayerController* _callerJHSPC = _otherPawnForId != nullptr ? Cast<AJHSPlayerController>(_otherPawnForId->GetController()) : nullptr;
	const int32 _callerAssignedId = _callerJHSPC != nullptr ? _callerJHSPC->GetAssignedPlayerId() : -1;

	const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;
	UE_LOG(LogTemp, Log, TEXT("[InteractFlow] ExecuteServerTriggerEnter - NetMode=%d OtherActor=%s CallerAssignedPlayerId=%d"),
		(int32)_netMode, *GetNameSafe(OtherActor), _callerAssignedId);

	// 끌어내리기: 서버에서 상태 갱신 후 대상 클라이언트에만 Client RPC
	if (_isInterrupt && _interacter != nullptr && _interacter != _foundInteracter)
	{
		_InterruptInteracter = _foundInteracter;
		if (_callerJHSPC != nullptr)
			_callerJHSPC->ClientInteractableTriggerEnter(this, E_INTERACT_TYPE::DumpThrow);
		return;
	}

	_interacter = _foundInteracter;
	E_INTERACT_TYPE _typeForEnter = _isWorldSpaceUI ? E_INTERACT_TYPE::Handle : E_INTERACT_TYPE::Seat;
	UE_LOG(LogTemp, Log, TEXT("[InteractFlow] ExecuteServerTriggerEnter - calling ClientInteractableTriggerEnter CallerAssignedPlayerId=%d InteractType=%d"), _callerAssignedId, (int32)_typeForEnter);
	// 대상 클라이언트에만 전달 (복제 타이밍 무관)
	if (_callerJHSPC != nullptr)
		_callerJHSPC->ClientInteractableTriggerEnter(this, _typeForEnter);
}

void UInteractableComponent::MulticastOnTriggerEnter_Implementation(int32 CallerAssignedPlayerId, E_INTERACT_TYPE InteractType)
{
	UWorld* _world = GetWorld();
	const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;
	UE_LOG(LogTemp, Log, TEXT("[InteractFlow] MulticastOnTriggerEnter_Implementation - ENTRY NetMode=%d CallerAssignedPlayerId=%d InteractType=%d"),
		(int32)_netMode, CallerAssignedPlayerId, (int32)InteractType);

	// CallerAssignedPlayerId에 해당하는 PC를 찾고, 그 PC가 로컬일 때만 처리 (로컬 PC 우선 조회에 의존하지 않음)
	AJHSPlayerController* _localController = nullptr;
	if (!FindLocalPlayerControllerByAssignedId(_world, CallerAssignedPlayerId, _localController))
	{
		UE_LOG(LogTemp, Log, TEXT("[InteractFlow] MulticastOnTriggerEnter_Implementation - SKIP (no local PC with AssignedPlayerId=%d, NetMode=%d)"),
			CallerAssignedPlayerId, (int32)_netMode);
		return;
	}

	APawn* _localPawn = _localController->GetPawn();
	UInteracterComponent* _foundInteracter = _localPawn != nullptr ? _localPawn->FindComponentByClass<UInteracterComponent>() : nullptr;
	if (_foundInteracter == nullptr)
	{
		UE_LOG(LogTemp, Log, TEXT("[InteractFlow] MulticastOnTriggerEnter_Implementation - SKIP (local Pawn or InteracterComponent=null, NetMode=%d)"), (int32)_netMode);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[InteractFlow] MulticastOnTriggerEnter_Implementation - calling OnInteractable CallerAssignedPlayerId=%d"), CallerAssignedPlayerId);
	_interacter = _foundInteracter;
	_foundInteracter->OnInteractable(this, InteractType);
}

void UInteractableComponent::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
		return;

	APawn* _otherPawn = Cast<APawn>(OtherActor);
	if (_otherPawn == nullptr)
		return;

	// 서버(권한 보유): 오버랩 해제가 서버에서 감지되면 서버에서 로직 실행 후 멀티캐스트
	AActor* _owner = GetOwner();
	if (_owner != nullptr && _owner->HasAuthority())
	{
		ExecuteServerTriggerExit(OtherActor);
		return;
	}

	// 클라이언트: Pawn 소유 InteracterComponent의 Server RPC로 알림
	if (!_otherPawn->IsLocallyControlled())
		return;

	UInteracterComponent* _foundInteracterExit = OtherActor->FindComponentByClass<UInteracterComponent>();
	if (_foundInteracterExit != nullptr)
		_foundInteracterExit->ServerReportTriggerExit(this);
}

void UInteractableComponent::ExecuteServerTriggerExit(AActor* OtherActor)
{
	if (OtherActor == nullptr)
		return;

	UInteracterComponent* _foundInteracter = OtherActor->FindComponentByClass<UInteracterComponent>();
	if (_foundInteracter == nullptr || _interacter != _foundInteracter)
		return;

	APawn* _otherPawn = Cast<APawn>(OtherActor);
	AJHSPlayerController* _callerJHSPC = _otherPawn != nullptr ? Cast<AJHSPlayerController>(_otherPawn->GetController()) : nullptr;
	const int32 _callerAssignedId = _callerJHSPC != nullptr ? _callerJHSPC->GetAssignedPlayerId() : -1;

	_interacter = nullptr;

	if (_callerJHSPC != nullptr)
		_callerJHSPC->ClientInteractableTriggerExit(this);

	if (!_isWorldSpaceUI && _isInteract)
		ChangeInteractState(false, _callerAssignedId);
}

void UInteractableComponent::MulticastOnTriggerExit_Implementation(int32 CallerAssignedPlayerId)
{
	UWorld* _world = GetWorld();
	AJHSPlayerController* _localController = nullptr;
	if (!FindLocalPlayerControllerByAssignedId(_world, CallerAssignedPlayerId, _localController))
		return;

	APawn* _localPawn = _localController->GetPawn();
	UInteracterComponent* _foundInteracter = _localPawn != nullptr ? _localPawn->FindComponentByClass<UInteracterComponent>() : nullptr;
	if (_foundInteracter == nullptr)
		return;

	_interacter = _foundInteracter;
	_foundInteracter->OnDisInteractable();
	_interacter = nullptr;
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

bool UInteractableComponent::TryInteract(APlayerController* CallerController, bool& OutIsInterupt, bool& IsCloseUI)
{
	OutIsInterupt = false;
	IsCloseUI = false;
	if (_interacter == nullptr)
		return false;

	if (CallerController == nullptr)
		return false;

	AJHSPlayerController* _jhsPC = Cast<AJHSPlayerController>(CallerController);
	if (_jhsPC == nullptr)
		return false;

	_isInteract = !_isInteract;
	const int32 _callerAssignedId = _jhsPC->GetAssignedPlayerId();
	ChangeInteractState(_isInteract, _callerAssignedId);

	IsCloseUI = _isInteract;
	if (IsCloseUI && _isWorldSpaceUI)
	{
		IsCloseUI = false;
	}
	
	return true;
}

void UInteractableComponent::ServerOpenWorldUI_Implementation(int32 CallerPlayerId, E_UI_TYPE UIType)
{
	// 서버에서 멀티캐스트로 전달 (호출자 클라이언트에서만 실제로 UI 열림)
	MulticastOpenWorldUI(CallerPlayerId, UIType);
}

void UInteractableComponent::MulticastOpenWorldUI_Implementation(int32 CallerPlayerId, E_UI_TYPE UIType)
{
	UWorld* _world = GetWorld();
	AJHSPlayerController* _localController = nullptr;
	if (!FindLocalPlayerControllerByAssignedId(_world, CallerPlayerId, _localController))
		return;

	UUIManager* _uiManagerPtr = _localController->GetUIManager();
	if (_uiManagerPtr == nullptr)
		return;

	AActor* _owner = GetOwner();
	if (_owner != nullptr)
		_uiManagerPtr->OpenUIInWorld(UIType, _owner, _worldUIRelativeLocation, _worldUIScale);
}

void UInteractableComponent::ServerCloseWorldUI_Implementation(int32 CallerPlayerId, E_UI_TYPE UIType)
{
	// 서버에서 멀티캐스트로 전달 (호출자 클라이언트에서만 실제로 UI 닫기)
	MulticastCloseWorldUI(CallerPlayerId, UIType);
}

void UInteractableComponent::MulticastCloseWorldUI_Implementation(int32 CallerPlayerId, E_UI_TYPE UIType)
{
	UWorld* _world = GetWorld();
	AJHSPlayerController* _localController = nullptr;
	if (!FindLocalPlayerControllerByAssignedId(_world, CallerPlayerId, _localController))
		return;

	UUIManager* _uiManagerPtr = _localController->GetUIManager();
	if (_uiManagerPtr != nullptr)
		_uiManagerPtr->CloseUI(UIType);
}

void UInteractableComponent::ChangeInteractState(bool IsInteract, int32 CallerPlayerId)
{
	_isInteract = IsInteract;

	if (_isInteract)
	{
		UUIBase* _openedUI = nullptr;
		if (_interactUIType != E_UI_TYPE::NONE)
		{
			if (_isWorldSpaceUI)
			{
				// 월드 공간 UI: 호출자 클라이언트에서만 열기 (RPC에서 CallerPlayerId로 필터)
				AActor* _owner = GetOwner();
				if (_owner != nullptr && _owner->HasAuthority())
				{
					MulticastOpenWorldUI(CallerPlayerId, _interactUIType);
				}
				else if (_owner != nullptr)
				{
					ServerOpenWorldUI(CallerPlayerId, _interactUIType);
				}
			}
			else
			{
				AJHSPlayerController* _localController = nullptr;
				if (GetOrCacheLocalPlayerController(_localController) && _localController->GetAssignedPlayerId() == CallerPlayerId)
				{
					UUIManager* _callerUIManager = _localController->GetUIManager();
					if (_callerUIManager != nullptr)
						_openedUI = _callerUIManager->OpenUI(_interactUIType);
				}
			}
		}
		OnInteractEnterAction.Broadcast(CallerPlayerId, _openedUI);
	}
	else
	{
		UUIBase* _closedUI = nullptr;
		if (_interactUIType != E_UI_TYPE::NONE)
		{
			if (_isWorldSpaceUI)
			{
				// 월드 공간 UI 닫기: 호출자 클라이언트에서만 닫기
				AActor* _owner = GetOwner();
				if (_owner != nullptr && _owner->HasAuthority())
				{
					MulticastCloseWorldUI(CallerPlayerId, _interactUIType);
				}
				else if (_owner != nullptr)
				{
					ServerCloseWorldUI(CallerPlayerId, _interactUIType);
				}
			}
			else
			{
				AJHSPlayerController* _localController = nullptr;
				if (GetOrCacheLocalPlayerController(_localController) && _localController->GetAssignedPlayerId() == CallerPlayerId)
				{
					UUIManager* _callerUIManager = _localController->GetUIManager();
					if (_callerUIManager != nullptr)
						_closedUI = _callerUIManager->CloseUI(_interactUIType);
				}
			}
		}
		OnInteractExitAction.Broadcast(CallerPlayerId, _closedUI);
	}
}

bool UInteractableComponent::TryGetUIManager(UUIManager*& OutUIManager)
{
	if (_uiManager != nullptr)
	{
		OutUIManager = _uiManager;
		return true;
	}

	AJHSPlayerController* _outPlayerController = nullptr;
	if (!GetOrCacheLocalPlayerController(_outPlayerController))
		return false;

	_uiManager = _outPlayerController->GetUIManager();
	if (_uiManager == nullptr)
		return false;

	OutUIManager = _uiManager;
	return true;
}