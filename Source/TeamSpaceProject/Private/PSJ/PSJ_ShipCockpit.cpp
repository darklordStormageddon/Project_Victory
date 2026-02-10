
#include "PSJ_ShipCockpit.h"
#include "PSJ_Character.h" 
#include "PSJ_Spaceship.h"
#include "YSH/TurretBase_GT.h"
#include "Net/UnrealNetwork.h" // [필수] 이 헤더가 맨 위에 있어야 합니다
#include "GameFramework/Pawn.h" // APawn 사용을 위해 필요
#include "GameFramework/Controller.h" // Controller 체크를 위해 필요
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

    // [!!! 핵심 누락 수정 !!!] 
    // 이 줄이 없으면 변수 동기화(Replicated)가 아예 작동하지 않습니다.
    bReplicates = true;
}

// [신규 추가] 변수 동기화 규칙 설정
void APSJ_ShipCockpit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // TargetSpaceship 변수를 서버 -> 클라이언트로 복제(Replication)합니다.
    DOREPLIFETIME(APSJ_ShipCockpit, TargetSpaceship);
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

void APSJ_ShipCockpit::ReceiveForceEjectRequest()
{
    // 1. 연결된 대상(우주선, 터렛 등)이 아예 세팅 안 된 경우 -> 무시
    if (!TargetSpaceship) return;

    // [핵심 안전장치] 대상 Pawn에 현재 빙의(Possess)한 컨트롤러(플레이어)가 있는가?
    // Controller가 nullptr라면, 타고 있는 사람이 없다는 뜻입니다.
    if (TargetSpaceship->GetController() == nullptr)
    {
        // 사람이 없으므로 하차 로직을 실행하지 않고 그냥 나갑니다.
        // 역으로 내가 탑승하는 로직도 없으므로 안전합니다.
        // UE_LOG(LogTemp, Log, TEXT("Cockpit: Seat is empty. No one to eject."));
        return;
    }

    // --- 사람이 있을 때만 아래 로직이 실행됩니다 ---

    // 2. 리플렉션을 이용해 연결된 Pawn의 'DisembarkCharacter' 함수 찾기
    static const FName DisembarkFuncName(TEXT("DisembarkCharacter"));
    UFunction* DisembarkFunc = TargetSpaceship->FindFunction(DisembarkFuncName);

    if (DisembarkFunc)
    {
        // 함수가 있으면 실행 (강제 하차)
        TargetSpaceship->ProcessEvent(DisembarkFunc, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("Cockpit: Force Eject Executed on %s"), *TargetSpaceship->GetName());
    }
    else
    {
        // 함수가 없으면 로그 출력
        UE_LOG(LogTemp, Error, TEXT("Cockpit: Target %s does NOT have 'DisembarkCharacter' function!"), *TargetSpaceship->GetName());
    }
}

void APSJ_ShipCockpit::AttemptBoarding(APSJ_Character* RequestingChar)
{
    if (!RequestingChar) return;
    if (!TargetSpaceship)
    {
        UE_LOG(LogTemp, Error, TEXT("Cockpit: TargetSpaceship is MISSING! Set it in Editor Details."));
        return;
    }

    // 터렛인 경우
    if (ATurretBase_GT* TargetTurret = Cast<ATurretBase_GT>(TargetSpaceship))
    {
        // 서버에게 "나 터렛 탈래"라고 요청 (캐릭터 내부에서 Server RPC 호출됨)
        RequestingChar->Server_RequestTurretBoarding(TargetTurret, this);
    }
    // 우주선인 경우
    else if (APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship))
    {
        Spaceship->LinkedCockpit = this;
        // 서버에게 "나 우주선 탈래"라고 요청
        RequestingChar->Server_RequestBoarding(Spaceship);
    }
}


void APSJ_ShipCockpit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 테스트용: F키를 누르기 전에 이미 변수가 들어왔는지 눈으로 확인
    if (GetWorld()->IsNetMode(NM_Client))
    {
        if (TargetSpaceship)
        {
            // 초록색: 연결 성공
            DrawDebugString(GetWorld(), GetActorLocation(), TEXT("Link OK"), nullptr, FColor::Green, 0.0f);
        }
        else
        {
            // 빨간색: 아직 변수 안 넘어옴 (이 상태면 탑승 불가)
            DrawDebugString(GetWorld(), GetActorLocation(), TEXT("Link NULL"), nullptr, FColor::Red, 0.0f);
        }
    }
}