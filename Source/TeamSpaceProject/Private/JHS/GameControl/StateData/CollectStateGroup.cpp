// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/GameControl/StateData/CollectStateGroup.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "KSM/DataTable/ToolDataTable.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/Event/EventManager.h"
#include "JHS/GameControl/CommonEnums.h"

// Sets default values for this component's properties
UCollectStateGroup::UCollectStateGroup()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCollectStateGroup::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCollectStateGroup::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCollectStateGroup::InitializeCollectState(TObjectPtr<AJHSGameState> GameState)
{
	_gameState = GameState;

	LoadCollectToolDataTable();
}

void UCollectStateGroup::UpdateCollectState()
{
	for (auto& _collectToolData : _collectToolDataMap)
	{
		ExecuteEventToolDurability(_collectToolData.Value);
	}
}

void UCollectStateGroup::RepairAllTool()
{
	for (auto& _collectToolData : _collectToolDataMap)
	{
		_collectToolData.Value.Durability.CurrentValue = _collectToolData.Value.Durability.MaxValue;
		ExecuteEventToolDurability(_collectToolData.Value);
	}
}

bool UCollectStateGroup::TrySelectTool(E_COLLECT_TOOL_TYPE CollectToolType)
{
	if (CollectToolType != E_COLLECT_TOOL_TYPE::NONE)
	{
		FCollectToolData* _outCollectToolData = nullptr;
		if (!TryGetCollectToolData(CollectToolType, _outCollectToolData))
			return false;
	}

	E_COLLECT_TOOL_TYPE _prevToolType = _selectedToolType;
	_selectedToolType = CollectToolType;
	ExecuteEventToolSelect(_prevToolType, _selectedToolType);
	return true;
}

bool UCollectStateGroup::TryUseTool(E_COLLECT_TOOL_TYPE CollectToolType, float DeltaTime, float& OutToolDamage)
{
	OutToolDamage = 0.0f;
	if (CollectToolType != _selectedToolType)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not selected tool type: %s"), *CommonEnums::GetEnum2FString<E_COLLECT_TOOL_TYPE>(CollectToolType));
		return false;
	}

	FCollectToolData* _outCollectToolData = nullptr;
	if (!TryGetCollectToolData(_selectedToolType, _outCollectToolData))
		return false;

	if (CollectToolType != E_COLLECT_TOOL_TYPE::Vacuum && _outCollectToolData->Durability.CurrentValue <= 0)
		return false;

	_outCollectToolData->Durability.CurrentValue -= CONSUME_DURABILITY * DeltaTime;
	if (_outCollectToolData->Durability.CurrentValue <= 0)
	{
		_outCollectToolData->Durability.CurrentValue = 0;
	}

	OutToolDamage = _outCollectToolData->ToolDamage;

	ExecuteEventToolDurability(*_outCollectToolData);
	return true;
}

void UCollectStateGroup::LoadCollectToolDataTable()
{
	_collectToolDataMap.Empty();

	TObjectPtr<UDataTable> _collectToolDataTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *ConstantLibrary::Resource.DataTable.COLLECT_TOOL_PATH));
	if (!_collectToolDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("UTurretStateGroup: Failed to load Room Data Table from path [%s]"), *ConstantLibrary::Resource.DataTable.TURRET_INFO_PATH);
		return;
	}

	TArray<FName> _rowNames = _collectToolDataTable->GetRowNames();
	for (const FName& _rowName : _rowNames)
	{
		FToolProperty* _toolInfo = _collectToolDataTable->FindRow<FToolProperty>(_rowName, TEXT(""));
		if (_toolInfo)
		{
			FCollectToolData _newCollectToolData;
			_newCollectToolData.CollectToolType = _toolInfo->ToolType;
			_newCollectToolData.Durability.MaxValue = _toolInfo->Durability;
			_newCollectToolData.Durability.CurrentValue = _newCollectToolData.Durability.MaxValue;
			_newCollectToolData.ToolDamage = _toolInfo->Damage;

			// Texture
			FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_COLLECT_TOOL_TYPE>(_newCollectToolData.CollectToolType);
			FString _texturePath = ConstantLibrary::Resource.Image.COLLECT_FOLDER_PATH + _fileName + "." + _fileName;
			UTexture2D* _loadedTexture = LoadObject<UTexture2D>(nullptr, *_texturePath);
			if (_loadedTexture != nullptr)
			{
				_newCollectToolData.CollectToolImage = _loadedTexture;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("UContainerStateGroup: Failed to load element texture: %s"), *_texturePath);
			}

			_collectToolDataMap.Add(_newCollectToolData.CollectToolType, _newCollectToolData);
		}
	}
}

bool UCollectStateGroup::TryGetCollectToolData(E_COLLECT_TOOL_TYPE CollectToolType, FCollectToolData*& OutCollectToolData)
{
	if (!_collectToolDataMap.Contains(CollectToolType))
		return false;

	OutCollectToolData = _collectToolDataMap.Find(CollectToolType);
	return OutCollectToolData != nullptr;
}

void UCollectStateGroup::ExecuteEventToolDurability(FCollectToolData CollectToolData)
{
	UEventOnChangeToolDurability* _event = NewObject<UEventOnChangeToolDurability>(this);
	_event->CollectToolData = CollectToolData;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeToolDurability>(_event);
}

void UCollectStateGroup::ExecuteEventToolSelect(E_COLLECT_TOOL_TYPE PrevToolType, E_COLLECT_TOOL_TYPE NextToolType)
{
	UEventOnChangeTool* _event = NewObject<UEventOnChangeTool>(this);
	_event->PrevToolType = PrevToolType;
	_event->NextToolType = NextToolType;
	_gameState->GetEventManager()->ExecuteEvent<UEventOnChangeTool>(_event);
}