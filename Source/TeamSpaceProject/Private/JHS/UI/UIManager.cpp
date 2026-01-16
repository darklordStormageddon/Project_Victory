// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIManager.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/Contant/ConstantLibrary.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
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

	// Panel UI인 경우 (100 미만) Viewport에 추가
	if ((int32)UIType < 100)
	{
		_ui->SetAsLastSibling();
		_ui->Open();
		//_openedUIStack.Push(_ui);
	}
	// Popup UI인 경우 (100 이상) 별도 처리 가능
	else
	{
		_ui->SetAsLastSibling();
		_ui->Open();
		//_openedUIStack.Push(_ui);
	}

	return _ui;
}

void UUIManager::CloseUI(E_UI_TYPE UIType)
{
	if (!_loadedUIDict.Contains(UIType))
	{
		UE_LOG(LogTemp, Warning, TEXT("UUIManager: UI [%d] is not loaded"), (int32)UIType);
		return;
	}

	UUIBase* _ui = _loadedUIDict[UIType];
	if (_ui != nullptr)
	{
		_ui->Close();
	}

	// 스택에서 제거
	/*TArray<TObjectPtr<UUIBase>> _tempStack;
	while (!_openedUIStack.IsEmpty())
	{
		TObjectPtr<UUIBase> _stackedUI = nullptr;
		_openedUIStack.Pop(_stackedUI);
		if (_stackedUI != _ui)
		{
			_tempStack.Add(_stackedUI);
		}
	}*/

	// 역순으로 다시 스택에 추가
	/*for (int32 i = _tempStack.Num() - 1; i >= 0; i--)
	{
		_openedUIStack.Push(_tempStack[i]);
	}*/
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
		FString _enumName = _enum->GetNameStringByValue((int64)UIType);
		_uiName = ConstantLibrary::Resource.UI.UI_WIDGET_HEADER + _enumName;
	}
	else
	{
		// Fallback: 직접 이름 매핑
		FString _widgetName = TEXT("");
		switch (UIType)
		{
		case E_UI_TYPE::UIPanelPlayerFPS:
			_widgetName = TEXT("UIPanelPlayerFPS");
			break;

		case E_UI_TYPE::UIPanelDriveSeat:
			_widgetName = TEXT("UIPanelDriveSeat");
			break;

		case E_UI_TYPE::UIPanelCollectSeat:
			_widgetName = TEXT("UIPanelCollectSeat");
			break;

		case E_UI_TYPE::UIPanelTurretSeat:
			_widgetName = TEXT("UIPanelTurretSeat");
			break;

		case E_UI_TYPE::UIPopupCommon:
			_widgetName = TEXT("UIPopupCommon");
			break;

		case E_UI_TYPE::UIPanelContainer:
			_widgetName = TEXT("UIPanelContainer");
			break;

		case E_UI_TYPE::UISystemSetting:
			_widgetName = TEXT("UISystemSetting");
			break;

		default:
			return TEXT("");
		}

		_uiName = ConstantLibrary::Resource.UI.UI_WIDGET_HEADER + _widgetName;
	}

	// 전체 경로 구성: /Game/Main/PS_JHS/Blueprint/UI/Popup/Common/WBP_UIPopupCommon.WBP_UIPopupCommon_C
	FString _fullPath = _basePath + _folderPath + _uiName;
	return _fullPath + TEXT(".") + _uiName + TEXT("_C");
}