#include "PSJ_ShipCockpit.h"
#include "PSJ_Character.h" 
#include "PSJ_Spaceship.h"
#include "YSH/TurretBase_GT.h"
#include "GameFramework/PlayerController.h"
#include "JHS/UI/UIBase.h"

// [1] 생성자 구현 (이 부분이 없어서 아까 에러가 난 것입니다)
APSJ_ShipCockpit::APSJ_ShipCockpit()
{
    // 틱 활성화
    PrimaryActorTick.bCanEverTick = true;

    // [핵심 해결책] 
    // 물리(Physics) 이동이 끝난 '뒤'에 틱을 실행해라!
    // -> 이 설정 덕분에 BP에서 그리는 디버그 라인도 밀리지 않고 딱 붙어 나옵니다.
    PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void APSJ_ShipCockpit::OnInteractEnter(TObjectPtr<UUIBase> OpenedUI)
{
    Super::OnInteractEnter(OpenedUI); // 부모의 기본 로직 실행

    if (TargetSpaceship) // 변수명은 TargetSpaceship이지만 실제로는 APawn* 타입
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn);

        if (!MyChar) return;

        // [분기 1] 대상이 터렛(TurretBase_GT)인 경우
        if (ATurretBase_GT* TargetTurret = Cast<ATurretBase_GT>(TargetSpaceship))
        {
            // 터렛용 탑승 요청 호출
            MyChar->Server_RequestTurretBoarding(TargetTurret, this);
            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting boarding to TURRET: %s"), *TargetTurret->GetName());
        }
        // [분기 2] 대상이 우주선(Spaceship)인 경우
        else if (APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship))
        {
            // 우주선에 조종석 연결 정보 전달
            Spaceship->LinkedCockpit = this;
            MyChar->Server_RequestBoarding(Spaceship);
            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting boarding to SPACESHIP: %s"), *Spaceship->GetName());
        }
    }
}

void APSJ_ShipCockpit::OnInteractExit(TObjectPtr<UUIBase> OpenedUI)
{
    // 1. TargetSpaceship이 유효한지 확인
    if (TargetSpaceship)
    {
        // 2. APawn 타입을 APSJ_Spaceship 타입으로 형변환
        APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship);

        // 3. 형변환에 성공했고, 현재 조종사가 있다면 종료(return) 처리
        if (Spaceship && Spaceship->GetCurrentPilot())
        {
            return;
        }
    }

    // 4. 조종사가 없거나 형변환에 실패한 경우 부모 로직 실행
    Super::OnInteractExit(OpenedUI);
}

void APSJ_ShipCockpit::SetTargetPawn(TObjectPtr<APawn> TargetPawn)
{
    TargetSpaceship = TargetPawn;
}