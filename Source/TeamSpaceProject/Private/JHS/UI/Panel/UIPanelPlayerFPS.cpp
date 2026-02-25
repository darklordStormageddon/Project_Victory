// UIPanelPlayerFPS.cpp
#include "JHS/UI/Panel/UIPanelPlayerFPS.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "JHS/Event/EventManager.h"
#include "Engine/World.h"

void UUIPanelPlayerFPS::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // NativeOnInitialized ???????? BindWidget?? ????? ???? ??????
    // ????????? ?? ?? ???
    if (IMG_Interact)
    {
        ChangeInteractable(E_INTERACT_TYPE::Idle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UIPanelPlayerFPS::NativeOnInitialized - IMG_Interact is nullptr, skipping initial ChangeInteractable"));
    }
}

void UUIPanelPlayerFPS::NativeDestruct()
{
    UnregisterEvent();
    Super::NativeDestruct();
}

void UUIPanelPlayerFPS::RegisterEvent()
{
    UWorld* _world = GetWorld();
    const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;

    UEventManager* EventMgr = GetEventManager();
    if (!EventMgr)
    {
        return;
    }

    if (_eventHandleOnChangeInteractType.IsValid())
    {
        return;
    }

    _eventHandleOnChangeInteractType = EventMgr->AddListener<UEventOnChangeInteractType>(
        [this](UEventOnChangeInteractType* Event)
        {
            if (!IsValid(this))
                return;
            OnChangeInteractType(Event);
        }
    );
}

void UUIPanelPlayerFPS::UnregisterEvent()
{
    if (!_eventHandleOnChangeInteractType.IsValid())
        return;

    UEventManager* EventMgr = GetEventManager();
    if (!EventMgr)
    {
        _eventHandleOnChangeInteractType.Reset();
        return;
    }

    EventMgr->DelListener<UEventOnChangeInteractType>(_eventHandleOnChangeInteractType);
    _eventHandleOnChangeInteractType.Reset();
}

void UUIPanelPlayerFPS::ChangeInteractable(E_INTERACT_TYPE InteractType)
{
    if (!IsValid(IMG_Interact))
    {
        return;
    }

    TObjectPtr<UTexture2D> Texture = nullptr;

    if (_interactTextureMap.Contains(InteractType))
    {
        Texture = _interactTextureMap.FindRef(InteractType);

        if (!IsValid(Texture))
        {
            _interactTextureMap.Remove(InteractType);
            Texture = nullptr;
        }
    }

    if (!IsValid(Texture))
    {
        FString FileName = ConstantLibrary::Resource.Image.TEXTURE_HEADER
            + CommonEnums::GetEnum2FString<E_INTERACT_TYPE>(InteractType);
        FString TexturePath = ConstantLibrary::Resource.Image.INTERACT_FOLDER_PATH
            + FileName + TEXT(".") + FileName;

        Texture = LoadObject<UTexture2D>(nullptr, *TexturePath);

        if (!IsValid(Texture))
        {
            UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS::ChangeInteractable - Texture not found at [%s]"), *TexturePath);
            return;
        }

        _interactTextureMap.Add(InteractType, Texture);
    }

    IMG_Interact->SetBrushFromTexture(Texture);
}

void UUIPanelPlayerFPS::OnChangeInteractType(UEventOnChangeInteractType* Event)
{
    if (!IsValid(Event))
    {
        return;
    }

    UWorld* _world = GetWorld();
    const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;
    AJHSPlayerController* _localJHSPC = Cast<AJHSPlayerController>(GetOwningPlayer());
    const int32 _localAssignedId = _localJHSPC ? _localJHSPC->GetAssignedPlayerId() : -1;

    if (Event->PlayerID != _localAssignedId)
    {
        return;
    }

    ChangeInteractable(Event->InteractType);
}