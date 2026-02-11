// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/EnumProperty.h"

// Sets default values for this component's properties
UUIManager::UUIManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UUIManager::BeginPlay()
{
	Super::BeginPlay();
}

void UUIManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 월드 공간 UI 컴포넌트 정리
	for (auto& Elem : _worldSpaceUIComponents)
	{
		if (Elem.Value.Get() != nullptr)
		{
			Elem.Value.Get()->DestroyComponent();
		}
	}
	_worldSpaceUIComponents.Empty();

	// 모든 로드된 UI 정리
	for (auto& Elem : _loadedUIDict)
	{
		if (Elem.Value != nullptr)
		{
			Elem.Value->Close();
			Elem.Value = nullptr;
		}
	}

	_loadedUIDict.Empty();

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void UUIManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

UUIBase* UUIManager::OpenUI(E_UI_TYPE UIType)
{
	UUIBase* _ui = LoadUIInternal(UIType);
	if (_ui == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: Failed to load UI [%d]"), (int32)UIType);
		return nullptr;
	}

	// Viewport에 추가 (일반 뷰포트 UI용)
	if (!_ui->IsInViewport())
	{
		_ui->AddToViewport(INT_MAX);
	}
	else
	{
		_ui->SetAsLastSibling();
	}

	_ui->Open();

	return _ui;
}

UUIBase* UUIManager::OpenUIInWorld(E_UI_TYPE UIType, AActor* OwnerActor, FVector RelativeLocation, float Scale)
{
	if (OwnerActor == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: OwnerActor is nullptr for WorldSpace UI [%d]"), (int32)UIType);
		return nullptr;
	}

	// UI 클래스 로드
	UUIBase* _ui = LoadUIInternal(UIType);
	if (_ui == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: Failed to load UI [%d]"), (int32)UIType);
		return nullptr;
	}

	TSubclassOf<UUIBase> _uiClass = _ui->GetClass();
	if (_uiClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: Failed to get UI class [%d]"), (int32)UIType);
		return nullptr;
	}

	// 이미 월드 공간에 UI가 있으면 제거
	if (_worldSpaceUIComponents.Contains(UIType))
	{
		TObjectPtr<UWidgetComponent> _existingComponent = _worldSpaceUIComponents[UIType];
		if (_existingComponent.Get() != nullptr)
		{
			_existingComponent.Get()->DestroyComponent();
		}
		_worldSpaceUIComponents.Remove(UIType);
	}

	// WidgetComponent 생성
	UWidgetComponent* _widgetComponent = NewObject<UWidgetComponent>(OwnerActor, UWidgetComponent::StaticClass(), FName(*FString::Printf(TEXT("WorldSpaceUI_%d"), (int32)UIType)));
	if (_widgetComponent == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: Failed to create WidgetComponent for UI [%d]"), (int32)UIType);
		return nullptr;
	}

	// Root Component에 부착
	USceneComponent* _rootComponent = OwnerActor->GetRootComponent();
	if (_rootComponent != nullptr)
	{
		_widgetComponent->SetupAttachment(_rootComponent);
	}

	OwnerActor->AddInstanceComponent(_widgetComponent);
	_widgetComponent->RegisterComponent();

	// WidgetComponent 설정
	_widgetComponent->SetWidgetClass(_uiClass);
	_widgetComponent->SetWidgetSpace(EWidgetSpace::World);
	_widgetComponent->SetVisibility(true);
	_widgetComponent->SetHiddenInGame(false);
	
	// 추가 설정: 월드 스페이스 UI가 보이도록
	_widgetComponent->SetPivot(FVector2D(0.5f, 0.5f)); // 중앙 피벗
	// 시선(Visibility) 트레이스를 맞추기 위한 충돌 설정
	// (UIBase를 상속받은 패널 내부에 UInteractableButton이 BindWidget으로 들어가는 구조이므로
	//  월드 스페이스 UI 전체에 대해 Visibility 라인 트레이스를 허용합니다.)
	_widgetComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 물리 충돌 X, 쿼리(라인 트레이스)만
	_widgetComponent->SetCollisionObjectType(ECC_WorldDynamic);
	_widgetComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	_widgetComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // Visibility만 Block
	_widgetComponent->SetGeometryMode(EWidgetGeometryMode::Plane); // 평면 모드
	_widgetComponent->SetBlendMode(EWidgetBlendMode::Transparent); // 투명 블렌드
	_widgetComponent->SetBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f)); // 투명 배경
	_widgetComponent->SetTwoSided(true); // 양면 렌더링
	
	// DrawSize를 고정 크기로 설정 (InitWidget 전에)
	_widgetComponent->SetDrawSize(FVector2D(1920.0f, 1080.0f));
	
	// Location과 Scale을 따로 설정 (Scale이 Location에 영향을 주지 않도록)
	_widgetComponent->SetRelativeLocation(RelativeLocation);
	_widgetComponent->SetRelativeRotation(FRotator::ZeroRotator);
	_widgetComponent->SetRelativeScale3D(FVector(Scale, Scale, Scale));
	
	// 위젯 강제 초기화
	_widgetComponent->InitWidget();

	// UI 위젯 가져오기
	UUserWidget* _widget = _widgetComponent->GetWidget();
	if (_widget != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UIManager] Widget created successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UIManager] Widget is null after InitWidget"));
	}

	// 맵에 저장
	_worldSpaceUIComponents.Add(UIType, _widgetComponent);

	// 디버그 로그: UI 생성 및 위치 정보
	FVector _relativeLocation = _widgetComponent->GetRelativeLocation();
	FVector _worldLocation = _widgetComponent->GetComponentLocation();
	FVector2D _finalDrawSize = _widgetComponent->GetDrawSize();
	FVector _finalScale = _widgetComponent->GetRelativeScale3D();

	// UI 타입 설정, 호스트 WidgetComponent 전달(충돌 기본 비활성 → Open() 시 UInteractableButton 있으면 활성화)
	UUIBase* _worldSpaceUI = Cast<UUIBase>(_widget);
	if (_worldSpaceUI != nullptr)
	{
		_worldSpaceUI->SetHostWidgetComponent(_widgetComponent);
		_worldSpaceUI->CurrentType = UIType;
		_worldSpaceUI->Open();
	}

	return _worldSpaceUI;
}

UUIBase* UUIManager::CloseUI(E_UI_TYPE UIType)
{
	UUIBase* _closedUI = nullptr;

	// 월드 공간 UI인 경우 WidgetComponent 제거
	if (_worldSpaceUIComponents.Contains(UIType))
	{
		TObjectPtr<UWidgetComponent> _widgetComponent = _worldSpaceUIComponents[UIType];
		if (_widgetComponent.Get() != nullptr)
		{
			// WidgetComponent가 생성한 위젯 가져오기
			UUserWidget* _widget = _widgetComponent.Get()->GetWidget();
			_closedUI = Cast<UUIBase>(_widget);

			_widgetComponent.Get()->DestroyComponent();
		}
		_worldSpaceUIComponents.Remove(UIType);
	}
	else
	{
		// 일반 뷰포트 UI
		if (!_loadedUIDict.Contains(UIType))
		{
			UE_LOG(LogTemp, Warning, TEXT("UUIManager: UI [%d] is not loaded"), (int32)UIType);
			return nullptr;
		}

		_closedUI = _loadedUIDict[UIType];
		if (_closedUI != nullptr)
		{
			_closedUI->Close();
		}
	}

	// 스택에서 제거
	/*TArray<TObjectPtr<UUIBase>> _tempStack;
	while (!_openedUIStack.IsEmpty())
	{
		TObjectPtr<UUIBase> _stackedUI = nullptr;
		_openedUIStack.Pop(_stackedUI);
		if (_stackedUI != _closedUI)
		{
			_tempStack.Add(_stackedUI);
		}
	}*/

	// 역순으로 다시 스택에 추가
	/*for (int32 i = _tempStack.Num() - 1; i >= 0; i--)
	{
		_openedUIStack.Push(_tempStack[i]);
	}*/

	return _closedUI;
}

void UUIManager::CloseAllUI()
{
	for (auto& Elem : _loadedUIDict)
	{
		if (Elem.Value != nullptr && !Elem.Value->GetName().Contains(TEXT("Loading"), ESearchCase::IgnoreCase))
		{
			Elem.Value->Close();
		}
	}

	//_openedUIStack.Empty();
}

UUIBase* UUIManager::GetUI(E_UI_TYPE UIType) const
{
	// 월드 공간 UI인 경우 WidgetComponent에서 위젯 가져오기
	if (_worldSpaceUIComponents.Contains(UIType))
	{
		TObjectPtr<UWidgetComponent> _widgetComponent = _worldSpaceUIComponents[UIType];
		if (_widgetComponent.Get() != nullptr)
		{
			UUserWidget* _widget = _widgetComponent.Get()->GetWidget();
			return Cast<UUIBase>(_widget);
		}
	}

	// 일반 뷰포트 UI
	if (_loadedUIDict.Contains(UIType))
	{
		return _loadedUIDict[UIType];
	}
	return nullptr;
}

UUIBase* UUIManager::LoadUIInternal(E_UI_TYPE UIType)
{
	if (_loadedUIDict.Contains(UIType))
	{
		return _loadedUIDict[UIType];
	}

	return InstantiateUI(UIType);
}

UUIBase* UUIManager::InstantiateUI(E_UI_TYPE UIType)
{
	UWorld* _world = GetWorld();
	if (_world == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: World is nullptr"));
		return nullptr;
	}

	APlayerController* _playerController = UGameplayStatics::GetPlayerController(_world, 0);
	if (_playerController == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: PlayerController is nullptr"));
		return nullptr;
	}

	// TSoftClassPtr에서 클래스 로드
	TSubclassOf<UUIBase> _uiClass = nullptr;
	if (_uiClassMap.Contains(UIType))
	{
		_uiClass = _uiClassMap[UIType].LoadSynchronous();
	}

	// 클래스 맵에 없으면 경로로 로드 시도
	if (_uiClass == nullptr)
	{
		FString _uiPath = GetUIPath(UIType);
		if (!_uiPath.IsEmpty())
		{
			_uiClass = LoadClass<UUIBase>(nullptr, *_uiPath);
		}
	}

	if (_uiClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: Cannot load UI class for type [%d]"), (int32)UIType);
		return nullptr;
	}

	UUIBase* _ui = CreateWidget<UUIBase>(_playerController, _uiClass);
	if (_ui == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UUIManager: Failed to create widget for type [%d]"), (int32)UIType);
		return nullptr;
	}

	_ui->CurrentType = UIType;
	_loadedUIDict.Add(UIType, _ui);

	return _ui;
}

FString UUIManager::GetUIPath(E_UI_TYPE UIType) const
{
	FString _basePath = ConstantLibrary::Resource.UI.BLUEPRINT_BASE_PATH;
	FString _folderPath = TEXT("");
	FString _uiName = TEXT("");

	// E_UI_TYPE에 따라 폴더 경로 결정
	int32 _uiTypeValue = (int32)UIType;
	if (_uiTypeValue < 100)
	{
		// Panel (0 ~ 99)
		_folderPath = ConstantLibrary::Resource.UI.UI_PANEL_FOLDER;
	}
	else if (_uiTypeValue < 200)
	{
		// Popup (100 ~ 199)
		_folderPath = ConstantLibrary::Resource.UI.UI_POPUP_FOLDER;
	}
	else
	{
		// System (200 ~)
		_folderPath = ConstantLibrary::Resource.UI.UI_SYSTEM_FOLDER;
	}

	// E_UI_TYPE 이름을 그대로 사용하여 위젯 이름 생성
	UEnum* _enum = FindObject<UEnum>(nullptr, TEXT("/Script/TeamSpaceProject.E_UI_TYPE"), true);
	if (_enum != nullptr)
	{
		FString _widgetName = CommonEnums::GetEnum2FString<E_UI_TYPE>(UIType);
		_uiName = ConstantLibrary::Resource.UI.UI_WIDGET_HEADER + _widgetName;
	}

	// 전체 경로 구성: /Game/Main/PS_JHS/Blueprint/UI/Popup/Common/WBP_UIPopupCommon.WBP_UIPopupCommon_C
	FString _fullPath = _basePath + _folderPath + _uiName;
	return _fullPath + TEXT(".") + _uiName + TEXT("_C");
}