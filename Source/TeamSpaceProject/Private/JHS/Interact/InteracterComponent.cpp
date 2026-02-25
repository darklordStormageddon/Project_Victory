// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteracterComponent.h"
#include "JHS/Interact/InteractableComponent.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/GameControl/JHSPlayerState.h"
#include "JHS/GameControl/StateData/PlayerStateGroup.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Event/EventManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UInteracterComponent::UInteracterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UInteracterComponent::BeginPlay()
{
	Super::BeginPlay();

	// InteracterComponent는 Pawn의 컴포넌트이므로 소유자 Pawn의 Controller에서 UIManager 가져오기
	APawn* _ownerPawn = Cast<APawn>(GetOwner());
	if (_ownerPawn == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInteracterComponent::BeginPlay - Owner is not APawn"));
		return;
	}

	_playerController = Cast<APlayerController>(_ownerPawn->GetController());
	if (_playerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInteracterComponent::BeginPlay - Controller is not APlayerController"));
		return;
	}

	// 로컬 플레이어인지 확인
	if (!_playerController->IsLocalPlayerController())
	{
		// UI는 로컬 플레이어에서만 표시
		return;
	}

	AJHSPlayerController* _jhsPlayerController = Cast<AJHSPlayerController>(_playerController);
	if (_jhsPlayerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInteracterComponent::BeginPlay - PlayerController is not AJHSPlayerController"));
		return;
	}

	_uiManager = _jhsPlayerController->GetUIManager();
	if (_uiManager == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UInteracterComponent::BeginPlay - UIManager is nullptr"));
		return;
	}

	_uiManager->OpenUI(_playerUI);
}

void UInteracterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInteracterComponent::OnInteractable(TObjectPtr<UInteractableComponent> Interactable, E_INTERACT_TYPE InteractType)
{
	UWorld* _world = GetWorld();
	const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;
	APawn* _ownerPawn = Cast<APawn>(GetOwner());
	const bool _bLocallyControlled = _ownerPawn != nullptr && _ownerPawn->IsLocallyControlled();

	if (_ownerPawn == nullptr || !_bLocallyControlled)
	{
		return;
	}

	_interactable = Interactable;
	if (_interactable != nullptr)
	{
		ExecuteEventOnChangeInteractType(InteractType);
	}
	else
	{
	}
}

void UInteracterComponent::OnDisInteractable()
{
	// 호출자(본 컴포넌트 소유 Pawn)를 로컬에서 조종 중인 클라이언트에서만 반응
	APawn* _ownerPawn = Cast<APawn>(GetOwner());
	if (_ownerPawn == nullptr || !_ownerPawn->IsLocallyControlled())
		return;

	_interactable = nullptr;
	if (_uiManager != nullptr)
	{
		_uiManager->OpenUI(_playerUI);
	}
	ExecuteEventOnChangeInteractType(E_INTERACT_TYPE::Idle);
}

bool UInteracterComponent::TryInteractInput(bool& OutIsInterupt, bool& OutIsInteractEnter)
{
	// 호출자 클라이언트에서만 반응 (로컬 플레이어가 아니면 무시)
	if (_playerController == nullptr || !_playerController->IsLocalPlayerController())
		return false;

	if (_interactable == nullptr)
		return false;

	if (!_interactable->TryInteract(_playerController, OutIsInterupt, OutIsInteractEnter))
		return false;

	// 월드 스페이스 UI일 때는 Open/Close를 멀티캐스트로만 처리. 여기서 _playerUI 토글 시 서버에서 닫자마자 다시 열리는 현상 방지.
	if (!_interactable->IsWorldSpaceUI() && _uiManager != nullptr)
	{
		if (OutIsInteractEnter)
		{
			_uiManager->CloseUI(_playerUI);
		}
		else
		{
			_uiManager->OpenUI(_playerUI);
		}
	}

	return true;
}

void UInteracterComponent::ServerReportTriggerEnter_Implementation(UInteractableComponent* Interactable)
{
	if (Interactable == nullptr)
		return;
	AActor* _ownerPawn = GetOwner();
	if (_ownerPawn == nullptr)
		return;
	Interactable->ExecuteServerTriggerEnter(_ownerPawn);
}

void UInteracterComponent::ServerReportTriggerExit_Implementation(UInteractableComponent* Interactable)
{
	if (Interactable == nullptr)
		return;
	AActor* _ownerPawn = GetOwner();
	if (_ownerPawn == nullptr)
		return;
	Interactable->ExecuteServerTriggerExit(_ownerPawn);
}

void UInteracterComponent::ExecuteEventOnChangeInteractType(E_INTERACT_TYPE InteractType)
{
	UWorld* _world = GetWorld();
	const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;

	UEventOnChangeInteractType* _event = NewObject<UEventOnChangeInteractType>(this);
	if (_event == nullptr)
	{
		return;
	}

	_event->InteractType = InteractType;
	AJHSPlayerController* _callerJHSPC = _playerController ? Cast<AJHSPlayerController>(_playerController) : nullptr;
	_event->PlayerID = _callerJHSPC ? _callerJHSPC->GetAssignedPlayerId() : -1;
	UEventManager::ExecuteEvent<UEventOnChangeInteractType>(_event);
}