// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/Panel/UIPanelPlayerFPS.h"
#include "JHS/GameControl/Contant/ConstantLibrary.h"

void UUIPanelPlayerFPS::NativeConstruct()
{
	Super::NativeConstruct();

	ChangeInteractable(E_INTERACT_TYPE::Idle);
}

void UUIPanelPlayerFPS::InitializeUI()
{
	ChangeInteractable(E_INTERACT_TYPE::Idle);
}

void UUIPanelPlayerFPS::ChangeInteractable(E_INTERACT_TYPE InteractType)
{
	if (IMG_Interact == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: IMG_Interact is nullptr"));
		return;
	}

	TObjectPtr<UTexture2D> _texture = nullptr;
	if (_interacTextureMap.Contains(InteractType))
	{
		_texture = _interacTextureMap.FindRef(InteractType);
	}
	else
	{
		//												  /Game/Main/PS_JHS/Resource/Images/Interact/Idle.uasset
		// C:/Users/jhsro/source/repos/Unreal/TeamSpace/Content/Main/PS_JHS/Resource/Images/Interact/Idle.uasset
		FString _texturePath = ConstantLibrary::Resource.Image.IMAGE_INTERACT_FOLDER_PATH + CommonEnums::GetFStringInteractEnum(InteractType) + "." + CommonEnums::GetFStringInteractEnum(InteractType);
		_texture = LoadObject<UTexture2D>(nullptr, *_texturePath);
		if (_texture == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS: Texture class is not found from [%s]"), *_texturePath);
			return;
		}

		_interacTextureMap.Add(InteractType, _texture);
	}

	IMG_Interact->SetBrushFromTexture(_texture);
}