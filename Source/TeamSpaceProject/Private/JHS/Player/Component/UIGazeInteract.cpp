// Fill out your copyright notice in the Description page of Project Settings.

#include "JHS/Player/Component/UIGazeInteract.h"

#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
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

	// 월드 히트 위치를 루트 위젯 로컬 좌표로 변환 (이 좌표계 기준으로 모든 히트 검사)
	FVector2D _localHitLocation = FVector2D::ZeroVector;
	_widgetComponent->GetLocalHitLocation(HitResult.ImpactPoint, _localHitLocation);

	const FGeometry _rootGeometry = _rootWidget->GetCachedGeometry();

	// 1) WidgetTree->GetAllWidgets로 블루프린트 위젯 수집
	TArray<UWidget*> _allWidgets;
	TSet<UWidget*> _seen;
	if (UWidgetTree* _tree = _rootWidget->WidgetTree)
	{
		_tree->GetAllWidgets(_allWidgets);
		for (UWidget* _w : _allWidgets)
		{
			_seen.Add(_w);
		}
	}

	// 2) BFS로 동적 추가 위젯(HB_Category->UPurchaseCategory->BTN_Category 등) 추가
	UWidget* _treeRoot = (_rootWidget->WidgetTree && _rootWidget->WidgetTree->RootWidget)
		? _rootWidget->WidgetTree->RootWidget
		: _rootWidget;
	TArray<UWidget*> _bfsQueue;
	_bfsQueue.Add(_treeRoot);
	for (int32 _i = 0; _i < _bfsQueue.Num(); ++_i)
	{
		UWidget* _w = _bfsQueue[_i];
		if (UPanelWidget* _panel = Cast<UPanelWidget>(_w))
		{
			const int32 _n = _panel->GetChildrenCount();
			for (int32 _c = 0; _c < _n; ++_c)
			{
				if (UWidget* _child = _panel->GetChildAt(_c))
				{
					_bfsQueue.Add(_child);
					if (!_seen.Contains(_child))
					{
						_seen.Add(_child);
						_allWidgets.Add(_child);
					}
				}
			}
		}
		else if (UUserWidget* _userW = Cast<UUserWidget>(_w))
		{
			// UUserWidget(예: UPurchaseCategory)은 UPanelWidget이 아니므로 GetChildAt 불가 → 내부 트리 루트를 큐에 추가
			if (_userW->WidgetTree && _userW->WidgetTree->RootWidget)
			{
				UWidget* _innerRoot = _userW->WidgetTree->RootWidget;
				_bfsQueue.Add(_innerRoot);
				if (!_seen.Contains(_innerRoot))
				{
					_seen.Add(_innerRoot);
					_allWidgets.Add(_innerRoot);
				}
			}
		}
	}

	// 3) 역순으로 히트 검사 (나중에 수집된/위에 그려진 버튼 우선)
	static constexpr float _slack = 2.0f;
	for (int32 _i = _allWidgets.Num() - 1; _i >= 0; --_i)
	{
		UInteractableButton* _button = Cast<UInteractableButton>(_allWidgets[_i]);
		if (_button == nullptr)
		{
			continue;
		}

		const FGeometry& _geometry = _button->GetCachedGeometry();
		const FVector2D _size = _geometry.GetLocalSize();
		if (_size.X <= 0.0f || _size.Y <= 0.0f)
		{
			continue;
		}

		FVector2D _min = _rootGeometry.AbsoluteToLocal(_geometry.LocalToAbsolute(FVector2D(0.0f, 0.0f)));
		FVector2D _max = _min;
		const FVector2D _corners[3] = {
			_geometry.LocalToAbsolute(FVector2D(_size.X, 0.0f)),
			_geometry.LocalToAbsolute(FVector2D(_size.X, _size.Y)),
			_geometry.LocalToAbsolute(FVector2D(0.0f, _size.Y))
		};
		for (int32 _j = 0; _j < 3; ++_j)
		{
			FVector2D _p = _rootGeometry.AbsoluteToLocal(_corners[_j]);
			_min.X = FMath::Min(_min.X, _p.X);
			_min.Y = FMath::Min(_min.Y, _p.Y);
			_max.X = FMath::Max(_max.X, _p.X);
			_max.Y = FMath::Max(_max.Y, _p.Y);
		}
		_min.X -= _slack;
		_min.Y -= _slack;
		_max.X += _slack;
		_max.Y += _slack;
		if (_localHitLocation.X >= _min.X && _localHitLocation.X <= _max.X &&
		    _localHitLocation.Y >= _min.Y && _localHitLocation.Y <= _max.Y)
		{
			return _button;
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

