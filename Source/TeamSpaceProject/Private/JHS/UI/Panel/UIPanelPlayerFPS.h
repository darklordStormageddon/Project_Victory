// UIPanelPlayerFPS.h
#pragma once
#include "CoreMinimal.h"
#include "JHS/GameControl/CommonEnums.h"
#include "JHS/Event/CommonEventBase.h"
#include "JHS/UI/UIBase.h"
#include "Components/Image.h"
#include "UIPanelPlayerFPS.generated.h"

UCLASS()
class UUIPanelPlayerFPS : public UUIBase
{
    GENERATED_BODY()

private:
    // UPROPERTY 추가 → GC에 의해 texture가 수집되는 것을 방지
    UPROPERTY()
    TMap<E_INTERACT_TYPE, TObjectPtr<UTexture2D>> _interactTextureMap;

    FDelegateHandle _eventHandleOnChangeInteractType;

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> IMG_Interact;

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override; // 소멸 시 이벤트 해제 보장
    void RegisterEvent() override;
    void UnregisterEvent() override;

private:
    void OnChangeInteractType(UEventOnChangeInteractType* Event);
    void ChangeInteractable(E_INTERACT_TYPE InteractType);
};