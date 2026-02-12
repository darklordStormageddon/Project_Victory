// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelPlayerFPS.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/Event/EventManager.h"

void UUIPanelPlayerFPS::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ChangeInteractable(E_INTERACT_TYPE::Idle);
}

void UUIPanelPlayerFPS::RegisterEvent()
{
	_eventHandleOnChangeInteractType = GetEventManager()->AddListener<UEventOnChangeInteractType>(
		[this](UEventOnChangeInteractType* Event)
		{
			OnChangeInteractType(Event);
		}
	);
}

void UUIPanelPlayerFPS::UnregisterEvent()
{
	if (_eventHandleOnChangeInteractType.IsValid())
	{
		GetEventManager()->DelListener<UEventOnChangeInteractType>(_eventHandleOnChangeInteractType);
		_eventHandleOnChangeInteractType.Reset();
	}
}

void UUIPanelPlayerFPS::ChangeInteractable(E_INTERACT_TYPE InteractType)
{
	if (IMG_Interact == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: IMG_Interact is nullptr"));
		return;
	}

	TObjectPtr<UTexture2D> _texture = nullptr;
	if (_interactTextureMap.Contains(InteractType))
	{
		_texture = _interactTextureMap.FindRef(InteractType);
	}
	else
	{
		FString _fileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER + CommonEnums::GetEnum2FString<E_INTERACT_TYPE>(InteractType);
		FString _texturePath = ConstantLibrary::Resource.Image.INTERACT_FOLDER_PATH + _fileName + "." + _fileName;
		_texture = LoadObject<UTexture2D>(nullptr, *_texturePath);
		if (_texture == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Texture class is not found from [%s]"), *_texturePath);
			return;
		}

		_interactTextureMap.Add(InteractType, _texture);
	}

	IMG_Interact->SetBrushFromTexture(_texture);
}

void UUIPanelPlayerFPS::OnChangeInteractType(UEventOnChangeInteractType* Event)
{
	if (Event == nullptr)
		return;

	ChangeInteractable(Event->InteractType);
}