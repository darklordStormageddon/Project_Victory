#pragma once

#include "CoreMinimal.h"
#include "PSJ_Character.h"
#include "JHS/Interact/InteractableActorBase.h" // 팀원의 베이스 클래스 헤더
#include "YSH/SolarWindManager.h"
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
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override; // 바인딩을 위해 오버라이드


    // [설정] 에디터에서 이 의자가 어떤 우주선을 조종할지 스포이드로 찍어줍니다. [수정] Replicated 속성 추가
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Link")
    APawn* TargetSpaceship = nullptr;

    // [신규] 동기화 설정을 위한 필수 오버라이드 함수
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // [신규] 블루프린트에서 "나 탈래!"라고 요청할 때 부르는 함수
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void AttemptBoarding(APSJ_Character* RequestingChar);


    // =========================================================
    // [신규] 태양풍 기능 고장 (Malfunction) 시스템
    // =========================================================
public:
    // 고장 상태 여부 (UI 표시 및 상호작용 차단용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsMalfunctioning, Category = "Malfunction")
    bool bIsMalfunctioning = false;

    // 태양풍 매니저의 방송을 수신하는 함수
    UFUNCTION()
    void HandleSolarWindEvent();

    // 이 의자가 태양풍에 의해 고장날 확률 (0.0 ~ 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Malfunction")
    float MalfunctionProbability = 0.5f;

    // 고장 지속 시간 (초) - 에디터에서 설정 (기본 20초)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Malfunction")
    float MalfunctionDuration = 20.0f;

    // 수리 가속 배율 (기본 1.2배, 합연산 적용)
    // 1명이 붙으면 1.2배, 2명이면 1.4배, 3명이면 1.6배 속도로 수리됨
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Malfunction", meta = (ClampMin = "1.0"))
    float RepairSpeedRate = 1.2f;

    // [서버용] 현재 남은 고장 시간
    float CurrentMalfunctionTimer = 0.0f;

    // [서버용] 현재 이 콕핏을 수리하고 있는 캐릭터 목록 (수리 속도 계산용)
    TArray<class APSJ_Character*> RepairingCharacters;

    // [외부 호출] 태양풍 매니저가 고장을 일으킬 때 부르는 함수
    void StartMalfunction();

    // [외부 호출] 캐릭터가 수리(좌클릭)를 시작/종료할 때 부르는 함수
    void AddRepairer(class APSJ_Character* Mechanic);
    void RemoveRepairer(class APSJ_Character* Mechanic);

    UFUNCTION()
    void OnRep_IsMalfunctioning();



    virtual void OnInteractExit(AActor* Caller, TObjectPtr<UUIBase> ClosedUI) override;

protected:
    virtual void OnInteractEnter(AActor* Caller, TObjectPtr<UUIBase> OpenedUI) override;

public:
    void SetTargetPawn(TObjectPtr<APawn> TargetPawn);

    // [신규] 외부에서 강제 하차 요청이 들어왔을 때 실행할 함수
    void ReceiveForceEjectRequest();
};