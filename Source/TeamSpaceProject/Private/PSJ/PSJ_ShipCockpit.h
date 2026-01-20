#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h" // 팀원의 베이스 클래스 헤더
#include "PSJ_ShipCockpit.generated.h"

class UUIBase;
class APSJ_Spaceship;

UCLASS()
class TEAMSPACEPROJECT_API APSJ_ShipCockpit : public AInteractableActorBase
{
    GENERATED_BODY()

public:
    // [설정] 에디터에서 이 의자가 어떤 우주선을 조종할지 스포이드로 찍어줍니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link")
    APSJ_Spaceship* TargetSpaceship;

    virtual void OnInteractExit(TObjectPtr<UUIBase> OpenedUI) override;

protected:
    // InteractableActorBase의 가상 함수 오버라이드
    virtual void OnInteractEnter(TObjectPtr<UUIBase> OpenedUI) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InteractableActorBase|Pawn")
    TObjectPtr<APawn> _taskPawn;


};