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
        UE_LOG(LogTemp, Error, TEXT("[InteractFlow] UIPanelPlayerFPS::RegisterEvent - EventManager=null listener NOT registered NetMode=%d"), (int32)_netMode);
        return;
    }

    if (_eventHandleOnChangeInteractType.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("[InteractFlow] UIPanelPlayerFPS::RegisterEvent - already registered skip NetMode=%d"), (int32)_netMode);
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
    UE_LOG(LogTemp, Log, TEXT("[InteractFlow] UIPanelPlayerFPS::RegisterEvent - UEventOnChangeInteractType listener registered NetMode=%d"), (int32)_netMode);
}

void UUIPanelPlayerFPS::UnregisterEvent()
{
    if (!_eventHandleOnChangeInteractType.IsValid())
        return;

    UEventManager* EventMgr = GetEventManager();
    if (!EventMgr)
    {
        // EventManager?? ??? ?????? ??? ??? ????
        _eventHandleOnChangeInteractType.Reset();
        return;
    }

    EventMgr->DelListener<UEventOnChangeInteractType>(_eventHandleOnChangeInteractType);
    _eventHandleOnChangeInteractType.Reset();
}

void UUIPanelPlayerFPS::ChangeInteractable(E_INTERACT_TYPE InteractType)
{
    UE_LOG(LogTemp, Log, TEXT("[InteractFlow] UIPanelPlayerFPS::ChangeInteractable - InteractType=%d"), (int32)InteractType);
    if (!IsValid(IMG_Interact))
    {
        UE_LOG(LogTemp, Error, TEXT("[InteractFlow] UIPanelPlayerFPS::ChangeInteractable - IMG_Interact is nullptr"));
        return;
    }

    TObjectPtr<UTexture2D> Texture = nullptr;

    if (_interactTextureMap.Contains(InteractType))
    {
        Texture = _interactTextureMap.FindRef(InteractType);

        // ????? ?????? GC ?????? ???????? ??? ?????
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
        UE_LOG(LogTemp, Warning, TEXT("[InteractFlow] UIPanelPlayerFPS::OnChangeInteractType - Event invalid"));
        return;
    }

    UWorld* _world = GetWorld();
    const ENetMode _netMode = _world != nullptr ? _world->GetNetMode() : NM_Standalone;
    AJHSPlayerController* _localJHSPC = Cast<AJHSPlayerController>(GetOwningPlayer());
    const int32 _localAssignedId = _localJHSPC ? _localJHSPC->GetAssignedPlayerId() : -1;

    UE_LOG(LogTemp, Log, TEXT("[InteractFlow] UIPanelPlayerFPS::OnChangeInteractType - ENTRY NetMode=%d EventPlayerID=%d InteractType=%d localAssignedId=%d"),
        (int32)_netMode, Event->PlayerID, (int32)Event->InteractType, _localAssignedId);

    if (Event->PlayerID != _localAssignedId)
    {
        UE_LOG(LogTemp, Log, TEXT("[InteractFlow] UIPanelPlayerFPS::OnChangeInteractType - SKIP (EventPlayerID=%d != localAssignedId=%d) NetMode=%d"),
            Event->PlayerID, _localAssignedId, (int32)_netMode);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("[InteractFlow] UIPanelPlayerFPS::OnChangeInteractType - applying InteractType=%d to UI NetMode=%d"), (int32)Event->InteractType, (int32)_netMode);
    ChangeInteractable(Event->InteractType);
}