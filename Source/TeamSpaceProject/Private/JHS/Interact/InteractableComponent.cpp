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
}


// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (_isDebugDraw && _collisionComponent != nullptr)
	{
		DrawDebugSphere(GetWorld(), _collisionComponent->GetComponentLocation(), _actorScale, 16, FColor::Yellow, false, DeltaTime * 1.01f);

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

void UInteractableComponent::InitializeUIInteractable(bool IsDebugDraw, float InteractRadius, E_INTERACT_TYPE InteractType, E_UI_TYPE InteractUIType, bool IsWorldSpaceUI, FVector WorldUIRelativeLocation, float WorldUIScale)
{
	_isDebugDraw = IsDebugDraw;
	_interactType = InteractType;
	_interactUIType = InteractUIType;
	_isWorldSpaceUI = IsWorldSpaceUI;
	_worldUIRelativeLocation = WorldUIRelativeLocation;
	_worldUIScale = WorldUIScale;

	// 월드 UI 멀티캐스트는 오너 액터가 각 클라이언트에 있어야 수신됨. 서버에서 복제 활성화.
	AActor* _owner = GetOwner();
	if (_owner != nullptr)
	{
		if (_isWorldSpaceUI && _owner->HasAuthority())
		{
			_owner->SetReplicates(true);
		}

		// X, Y, Z 중 가장 작은 값을 사용
		FVector _actorScale3D = _owner->GetActorRelativeScale3D();
		_actorScale3D = FVector(FMath::Min(_actorScale3D.X, FMath::Min(_actorScale3D.Y, _actorScale3D.Z)));
		_actorScale = _actorScale3D.X * InteractRadius;
		if (_actorScale <= 0.0f)
		{
			_actorScale = 100.0f;
		}

		_collisionComponent = NewObject<USphereComponent>(_owner, USphereComponent::StaticClass(), TEXT("CollisionComponent"));
		if (_collisionComponent != nullptr)
		{
			_collisionComponent->SetSphereRadius(_actorScale);
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

	// 월드 UI: 열림/닫힘은 서버에서만 처리. 클라이언트는 토글 요청만 보내고 상태 갱신·멀티캐스트는 서버가 담당.
	if (_isWorldSpaceUI)
	{
		_jhsPC->ServerRequestToggleWorldUI(this);
		IsCloseUI = false;
		return true;
	}

	_isInteract = !_isInteract;
	const int32 _callerAssignedId = _jhsPC->GetAssignedPlayerId();
	ChangeInteractState(_isInteract, _callerAssignedId, CallerController);

	IsCloseUI = _isInteract;
	return true;
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
	const bool _isAuthority = _owner != nullptr && _owner->HasAuthority();

	if (_isAuthority)
	{
		ExecuteServerTriggerEnter(OtherActor);
		return;
	}

	if (!_otherPawn->IsLocallyControlled())
		return;

	_foundInteracter->ServerReportTriggerEnter(this);
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

	// 끌어내리기: 서버에서 상태 갱신 후 대상 클라이언트에만 Client RPC
	if (_isInterrupt && _interacter != nullptr && _interacter != _foundInteracter)
	{
		_InterruptInteracter = _foundInteracter;
		if (_callerJHSPC != nullptr)
		{
			_callerJHSPC->ClientInteractableTriggerEnter(this, E_INTERACT_TYPE::DumpThrow);
		}

		return;
	}

	_interacter = _foundInteracter;
	E_INTERACT_TYPE _typeForEnter = _isWorldSpaceUI ? E_INTERACT_TYPE::Handle : E_INTERACT_TYPE::Seat;
	// 대상 클라이언트에만 전달 (복제 타이밍 무관)
	if (_callerJHSPC != nullptr)
	{
		_callerJHSPC->ClientInteractableTriggerEnter(this, _typeForEnter);
	}
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
	{
		_callerJHSPC->ClientInteractableTriggerExit(this);
	}

	if (!_isWorldSpaceUI && _isInteract)
	{
		ChangeInteractState(false, _callerAssignedId, _callerJHSPC);
	}
	else if (_isWorldSpaceUI && _isInteract)
	{
		ChangeInteractState(false, _callerAssignedId, _callerJHSPC);
	}
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

void UInteractableComponent::MulticastOnTriggerEnter_Implementation(int32 CallerAssignedPlayerId, E_INTERACT_TYPE InteractType)
{
	UWorld* _world = GetWorld();
	const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;

	// CallerAssignedPlayerId에 해당하는 PC를 찾고, 그 PC가 로컬일 때만 처리 (로컬 PC 우선 조회에 의존하지 않음)
	AJHSPlayerController* _localController = nullptr;
	if (!FindLocalPlayerControllerByAssignedId(_world, CallerAssignedPlayerId, _localController))
	{
		return;
	}

	APawn* _localPawn = _localController->GetPawn();
	UInteracterComponent* _foundInteracter = _localPawn != nullptr ? _localPawn->FindComponentByClass<UInteracterComponent>() : nullptr;
	if (_foundInteracter == nullptr)
	{
		return;
	}
	_interacter = _foundInteracter;
	_foundInteracter->OnInteractable(this, InteractType);
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

void UInteractableComponent::AuthorityToggleWorldUI()
{
	AActor* _owner = GetOwner();
	if (_owner == nullptr || !_owner->HasAuthority())
		return;

	_isInteract = !_isInteract;
	if (_isInteract)
	{
		MulticastOpenWorldUI(_interactUIType, _owner, _worldUIRelativeLocation, _worldUIScale);
	}
	else
	{
		MulticastCloseWorldUI(_interactUIType);
	}
}

void UInteractableComponent::MulticastOpenWorldUI_Implementation(E_UI_TYPE UIType, AActor* OwnerActor, FVector RelativeLocation, float Scale)
{
	if (OwnerActor == nullptr) return;
	AJHSPlayerController* _localController = nullptr;
	if (!GetOrCacheLocalPlayerController(_localController)) return;
	UUIManager* _localUIManager = _localController->GetUIManager();
	if (_localUIManager != nullptr)
		_localUIManager->OpenUIInWorldLocal(UIType, OwnerActor, RelativeLocation, Scale);
}

void UInteractableComponent::MulticastCloseWorldUI_Implementation(E_UI_TYPE UIType)
{
	AJHSPlayerController* _localController = nullptr;
	if (!GetOrCacheLocalPlayerController(_localController)) return;
	UUIManager* _localUIManager = _localController->GetUIManager();
	if (_localUIManager != nullptr)
		_localUIManager->CloseWorldUILocal(UIType);
}

void UInteractableComponent::ChangeInteractState(bool IsInteract, int32 CallerPlayerId, APlayerController* CallerController)
{
	_isInteract = IsInteract;

	// 월드 UI 열기/닫기는 서버에서만 AuthorityToggleWorldUI 또는 트리거 이탈 시 여기(서버)에서 처리. 클라이언트는 TryInteract에서 토글 요청만 보냄.
	if (_isInteract)
	{
		UUIBase* _openedUI = nullptr;
		if (_interactUIType != E_UI_TYPE::NONE && !_isWorldSpaceUI)
		{
			// 일반 뷰포트 UI: 로컬 플레이어만 표시
			AJHSPlayerController* _localController = nullptr;
			if (GetOrCacheLocalPlayerController(_localController) && _localController->GetAssignedPlayerId() == CallerPlayerId)
			{
				UUIManager* _callerUIManager = _localController->GetUIManager();
				if (_callerUIManager != nullptr)
					_openedUI = _callerUIManager->OpenUI(_interactUIType);
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
				// 트리거 이탈 등 서버 전용 경로에서만 호출됨. 서버가 멀티캐스트로 닫기.
				if (GetOwner() != nullptr && GetOwner()->HasAuthority())
					MulticastCloseWorldUI(_interactUIType);
			}
			else
			{
				// 일반 뷰포트 UI 닫기: 로컬 플레이어만
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