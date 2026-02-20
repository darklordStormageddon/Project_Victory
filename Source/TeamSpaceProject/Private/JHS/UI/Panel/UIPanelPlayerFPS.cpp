// UIPanelPlayerFPS.cpp
#include "JHS/UI/Panel/UIPanelPlayerFPS.h"
#include "JHS/GameControl/Constant/ConstantLibrary.h"
#include "JHS/Event/EventManager.h"

void UUIPanelPlayerFPS::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // NativeOnInitialized 시점에는 BindWidget이 완료되어 있어야 하지만
    // 방어적으로 체크 후 호출
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
    // Widget이 파괴될 때 이벤트 핸들이 남아있으면 명시적으로 해제
    UnregisterEvent();
    Super::NativeDestruct();
}

void UUIPanelPlayerFPS::RegisterEvent()
{
    // EventManager nullptr 체크
    UEventManager* EventMgr = GetEventManager();
    if (!EventMgr)
    {
        UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS::RegisterEvent - EventManager is nullptr"));
        return;
    }

    // 이미 등록된 핸들이 있으면 중복 등록 방지
    if (_eventHandleOnChangeInteractType.IsValid())
    {
        return;
    }

    _eventHandleOnChangeInteractType = EventMgr->AddListener<UEventOnChangeInteractType>(
        [this](UEventOnChangeInteractType* Event)
        {
            // this가 유효한지 체크 (WeakPtr 패턴 또는 IsValid 활용)
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
        // EventManager가 이미 없어진 경우 핸들만 초기화
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
        UE_LOG(LogTemp, Error, TEXT("UIPanelPlayerFPS::ChangeInteractable - IMG_Interact is nullptr"));
        return;
    }

    TObjectPtr<UTexture2D> Texture = nullptr;

    if (_interactTextureMap.Contains(InteractType))
    {
        Texture = _interactTextureMap.FindRef(InteractType);

        // 캐시된 텍스처가 GC 등으로 무효화됐을 경우 재로드
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
        return;

    ChangeInteractable(Event->InteractType);
}