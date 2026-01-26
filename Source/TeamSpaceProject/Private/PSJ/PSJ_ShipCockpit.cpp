#include "PSJ_ShipCockpit.h"
#include "PSJ_Character.h" 
#include "PSJ_Spaceship.h"
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
    // 1. 부모 로직 실행
    Super::OnInteractEnter(OpenedUI);

    // 2. 우주선 연결 및 탑승 처리
    if (TargetSpaceship)
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();

        if (APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn))
        {
            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting Boarding..."));

            TargetSpaceship->LinkedCockpit = this;

            // [변경 핵심] 직접 Possess 하지 않고 캐릭터의 서버 RPC 함수를 호출
            MyChar->Server_RequestBoarding(TargetSpaceship);

        }
    }
}

void APSJ_ShipCockpit::OnInteractExit(TObjectPtr<UUIBase> OpenedUI)
{
    // [예외 처리] 탑승 중(=파일럿 있음)이라면 UI 끄기 무시
    if (TargetSpaceship && TargetSpaceship->GetCurrentPilot())
    {
        return;
    }

    Super::OnInteractExit(OpenedUI);
}