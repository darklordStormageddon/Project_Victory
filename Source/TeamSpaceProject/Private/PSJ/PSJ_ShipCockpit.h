#pragma once

#include "CoreMinimal.h"
#include "PSJ_Character.h"
#include "JHS/Interact/InteractableActorBase.h" // 팀원의 베이스 클래스 헤더
#include "PSJ_ShipCockpit.generated.h"



class UUIBase;
class APawn;


UCLASS()
class TEAMSPACEPROJECT_API APSJ_ShipCockpit : public AInteractableActorBase
{
    GENERATED_BODY()



public:
    // [1] 생성자 선언 (Tick 설정을 위해 필수)
    APSJ_ShipCockpit();

    // [설정] 에디터에서 이 의자가 어떤 우주선을 조종할지 스포이드로 찍어줍니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Link")
    APawn* TargetSpaceship = nullptr;

    // [신규] 블루프린트에서 "나 탈래!"라고 요청할 때 부르는 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void AttemptBoarding(APSJ_Character* RequestingChar);

    virtual void OnInteractExit(TObjectPtr<UUIBase> OpenedUI) override;

protected:
    virtual void OnInteractEnter(TObjectPtr<UUIBase> OpenedUI) override;
        AJHSGameState* _outGameState = nullptr;

public:
    void SetTargetPawn(TObjectPtr<APawn> TargetPawn);

    // [신규] 외부에서 강제 하차 요청이 들어왔을 때 실행할 함수
    void ReceiveForceEjectRequest();
};