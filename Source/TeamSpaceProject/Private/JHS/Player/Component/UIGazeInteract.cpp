// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/Player/Component/UIGazeInteract.h"

#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Layout/Geometry.h"
#include "JHS/UI/Interact/InteractableButton.h"
#include "TeamSpaceProject/TeamSpaceProjectCharacter.h"

UIGazeInteract::UIGazeInteract(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UIGazeInteract::BeginPlay()
{
	Super::BeginPlay();
}

void UIGazeInteract::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!_enabled)
	{
		_UpdateFocus(nullptr);
		return;
	}

	UWorld* _world = GetWorld();
	if (_world == nullptr)
	{
		_UpdateFocus(nullptr);
		return;
	}

	UCameraComponent* _camera = _ResolveCamera();

	const FVector _start = (_camera != nullptr) ? _camera->GetComponentLocation() : GetOwner()->GetActorLocation();
	const FVector _direction = (_camera != nullptr) ? _camera->GetForwardVector() : GetOwner()->GetActorForwardVector();
	const FVector _end = _start + (_direction * _distance);

	FHitResult _hitResult;
	FCollisionQueryParams _params(SCENE_QUERY_STAT(UIGazeInteract), false);
	_params.AddIgnoredActor(GetOwner());

	const bool _bHit = _world->LineTraceSingleByChannel(_hitResult, _start, _end, _traceChannel, _params);

#if WITH_EDITOR
	if (_drawDebug)
	{
		const FVector _debugEnd = _bHit ? _hitResult.ImpactPoint : _end;
		DrawDebugLine(_world, _start, _debugEnd, _bHit ? FColor::Green : FColor::Red, false, 0.0f, 0, 1.0f);
	}
#endif

	UInteractableButton* _hitButton = nullptr;
	if (_bHit)
	{
		_hitButton = _FindInteractableFromHit(_hitResult);
	}

	_UpdateFocus(_hitButton);
}

void UIGazeInteract::ClickFocused()
{
	if (!_enabled)
	{
		return;
	}

	UInteractableButton* _button = _focusedButton.Get();
	if (_button == nullptr)
	{
		return;
	}

	_button->Click();
}

void UIGazeInteract::SetEnabled(bool bEnabled)
{
	_enabled = bEnabled;
	if (!_enabled)
	{
		_UpdateFocus(nullptr);
	}
}

UCameraComponent* UIGazeInteract::_ResolveCamera() const
{
	const AActor* _ownerActor = GetOwner();
	if (_ownerActor == nullptr)
	{
		return nullptr;
	}

	// 프로젝트 기본 캐릭터면 FollowCamera를 우선 사용
	if (const ATeamSpaceProjectCharacter* _baseCharacter = Cast<ATeamSpaceProjectCharacter>(_ownerActor))
	{
		return _baseCharacter->GetFollowCamera();
	}

	// 그 외 Pawn은 CameraComponent를 검색
	return _ownerActor->FindComponentByClass<UCameraComponent>();
}

UInteractableButton* UIGazeInteract::_FindInteractableFromHit(const FHitResult& HitResult) const
{
	UWidgetComponent* _widgetComponent = Cast<UWidgetComponent>(HitResult.GetComponent());

	if (_widgetComponent == nullptr && HitResult.GetActor() != nullptr)
	{
		_widgetComponent = HitResult.GetActor()->FindComponentByClass<UWidgetComponent>();
	}

	if (_widgetComponent == nullptr)
	{
		return nullptr;
	}

	UUserWidget* _rootWidget = _widgetComponent->GetWidget();
	if (_rootWidget == nullptr)
	{
		return nullptr;
	}

	// 월드 히트 위치를 위젯 로컬 좌표로 변환
	FVector2D _localHitLocation = FVector2D::ZeroVector;
	_widgetComponent->GetLocalHitLocation(HitResult.ImpactPoint, _localHitLocation);

	// 패널(UIBase) 내부에서, 실제 히트 위치를 포함하는 UInteractableButton만 선택
	if (UWidgetTree* _widgetTree = _rootWidget->WidgetTree)
	{
		TArray<UWidget*> _allWidgets;
		_widgetTree->GetAllWidgets(_allWidgets);

		for (UWidget* _widget : _allWidgets)
		{
			UInteractableButton* _button = Cast<UInteractableButton>(_widget);
			if (_button == nullptr)
			{
				continue;
			}

			const FGeometry& _geometry = _button->GetCachedGeometry();
			if (_geometry.IsUnderLocation(_localHitLocation))
			{
				return _button;
			}
		}
	}

	return nullptr;
}

void UIGazeInteract::_UpdateFocus(UInteractableButton* NewButton)
{
	UInteractableButton* _prev = _focusedButton.Get();
	if (_prev == NewButton)
	{
		return;
	}

	if (_prev != nullptr)
	{
		_prev->Unfocus();
	}

	_focusedButton = NewButton;

	if (NewButton != nullptr)
	{
		NewButton->Focus();
	}
}

