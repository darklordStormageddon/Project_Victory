
#include "PSJ_ShipCockpit.h"
#include "Kismet/GameplayStatics.h"
#include "PSJ_Character.h" 
#include "PSJ_Spaceship.h"
#include "YSH/TurretBase_GT.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "JHS/UI/UIBase.h"

APSJ_ShipCockpit::APSJ_ShipCockpit()
{
    PrimaryActorTick.bCanEverTick = true;

    // [중요 설정] 
    // 물리(Physics) 시뮬레이션 직후 '다음'에 이 틱을실행함!
    // -> 즉, 우주선 본체가 BP에서 움직인 뒤에야 콕핏도 따라가서 동기화 할 수 있게 만듦.
    PrimaryActorTick.TickGroup = TG_PostPhysics;

    // [!!! 멀티 플레이 대비 !!!] 
    // 이 액터 자체가 네트워크 복제(Replicated)가 되게 설정해야 동기화됨.
    bReplicates = true;
}

void APSJ_ShipCockpit::BeginPlay()
{
    Super::BeginPlay();

    // [서버] 솔라윈드에 이벤트 바인딩 등록.
    if (HasAuthority())
    {
        // 1. 월드 안에 있는 매니저를 검색함.
        // (현재 월드에서 1개만 존재한다고 GetActorOfClass 사용)
        AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASolarWindManager::StaticClass());

        if (ASolarWindManager* WindManager = Cast<ASolarWindManager>(ManagerActor))
        {
            // 2. 매니저의 델리게이트에 연결(Bind)함.
            // "태양풍이 터지면, 우리의 함수(HandleSolarWindEvent)를 호출하세요"
            WindManager->OnSolarWindImpact.AddDynamic(this, &APSJ_ShipCockpit::HandleSolarWindEvent);

            UE_LOG(LogTemp, Log, TEXT("[Cockpit] Successfully bound to SolarWindManager."));
        }
    }
}

// [이벤트] 태양풍 발생 콜백 (Delegate가 자동 호출함)
void APSJ_ShipCockpit::HandleSolarWindEvent()
{
    // 서버에서만 실행 할 것 (클라에선X)
    if (!HasAuthority()) return;

    // 이미 고장 상태라면 또 다시 체크 안함(중복방지)
    if (bIsMalfunctioning) return;

    // 1. 확률 체크 (랜덤값 뽑기)
    float DiceRoll = FMath::FRand(); // 0.0 ~ 1.0 범위

    if (DiceRoll <= MalfunctionProbability)
    {
        // 걸렸다! 고장 상태 돌입
        UE_LOG(LogTemp, Warning, TEXT("[Cockpit] Hit by Solar Wind! (Roll: %.2f <= Prob: %.2f)"), DiceRoll, MalfunctionProbability);
        StartMalfunction();
    }
    else
    {
        // 무사 통과
        UE_LOG(LogTemp, Log, TEXT("[Cockpit] Survived Solar Wind. (Roll: %.2f > Prob: %.2f)"), DiceRoll, MalfunctionProbability);
    }
}

// [멀티 플레이] 동기화 속성을 등록
void APSJ_ShipCockpit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // TargetSpaceship 변수를 동기화 -> 클라이언트에 복제(Replication)됨.
    DOREPLIFETIME(APSJ_ShipCockpit, TargetSpaceship);

    // [중요] 고장 상태 복제함 (모든 클라가 고장 화면을 볼 수 있게)
    DOREPLIFETIME(APSJ_ShipCockpit, bIsMalfunctioning);
}

void APSJ_ShipCockpit::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{

    // [고장] 고장 중일 때
    if (bIsMalfunctioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Cockpit] System Error! Repair required before boarding."));
        // 추후 "시스템 오류입니다" 팝업 메시지 띄우고 탑승 거절할 수 있습니다.
        return;
    }

    Super::OnInteractEnter(CallerPlayerId, OpenedUI);

    if (TargetSpaceship)
    {
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn);

        if (!MyChar) return;

        // [케이스 1] 대상이 터렛(TurretBase_GT)인 경우
        if (ATurretBase_GT* TargetTurret = Cast<ATurretBase_GT>(TargetSpaceship))
        {
            // 터렛에 탑승 요청 전송
            MyChar->Server_RequestTurretBoarding(TargetTurret, this);
            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting boarding to TURRET: %s"), *TargetTurret->GetName());
        }
        // [케이스 2] 대상이 우주선(Spaceship)인 경우
        else if (APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship))
        {
            // 우주선에 콕핏을 연결 먼저 해줌
            Spaceship->LinkedCockpit = this;
            MyChar->Server_RequestBoarding(Spaceship);
            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting boarding to SPACESHIP: %s"), *Spaceship->GetName());
        }
    }
}

// [3] 고장 시작 (SolarWindManager가 호출)
void APSJ_ShipCockpit::StartMalfunction()
{
    if (!HasAuthority()) return; // 서버에서만 실행

    // 이미 고장 중이면 타이머만 갱신 (중복 방지 용도)
    if (bIsMalfunctioning)
    {
        CurrentMalfunctionTimer = MalfunctionDuration;
        return;
    }

    // 1. 탑승자를 강제로 추방 (있으면 내보냄)
    ReceiveForceEjectRequest();

    // 2. 고장 시작
    bIsMalfunctioning = true;
    CurrentMalfunctionTimer = MalfunctionDuration;
    RepairingCharacters.Empty(); // 수리 목록 초기화

    // 3. 시각 효과 (OnRep 호출)
    OnRep_IsMalfunctioning();

    UE_LOG(LogTemp, Error, TEXT("[Cockpit] MALFUNCTION STARTED! Timer: %.1f"), MalfunctionDuration);
}


void APSJ_ShipCockpit::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
    // 1. TargetSpaceship이 존재하면 체크
    if (TargetSpaceship)
    {
        // 2. APawn 형태를 APSJ_Spaceship 타입으로 캐스팅
        APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship);

        // 3. 우주선이 존재하고, 현재 조종사가 있으면 종료(return) 처리
        if (Spaceship && Spaceship->GetCurrentPilot())
        {
            return;
        }
    }

    // 4. 조종사가 없거나 우주선이 아니면 정상 종료 로직 실행
    Super::OnInteractExit(CallerPlayerId, ClosedUI);
}

void APSJ_ShipCockpit::SetTargetPawn(TObjectPtr<APawn> TargetPawn)
{
    TargetSpaceship = TargetPawn;
}

void APSJ_ShipCockpit::ReceiveForceEjectRequest()
{
    // 1. 대상이 없거나(우주선, 터렛 등)가 세팅 안된 경우 -> 종료
    if (!TargetSpaceship) return;

    // [중요 최적화] 만약 Pawn에 아무도 타고있지(Possess)가 않다면(컨트롤러가 없다면) 넘어감
    // Controller가 nullptr이면, 좌석 안에 탑승자가 없다는 뜻이므로.
    if (TargetSpaceship->GetController() == nullptr)
    {
        // 사람이 안타고있으면 굳이 강제로 내보낼필요 없는 거임 당연함.
        // 로그도 자주 나오면 스팸이 되므로 주석처리함.
        // UE_LOG(LogTemp, Log, TEXT("Cockpit: Seat is empty. No one to eject."));
        return;
    }

    // --- 탑승자가 있는 경우 여기부터 실행됨 ---

    // 2. 리플렉션으로 동적으로 대상 Pawn의 'DisembarkCharacter' 함수 호출
    static const FName DisembarkFuncName(TEXT("DisembarkCharacter"));
    UFunction* DisembarkFunc = TargetSpaceship->FindFunction(DisembarkFuncName);

    if (DisembarkFunc)
    {
        // 함수를 찾아서 호출 (강제 하선)
        TargetSpaceship->ProcessEvent(DisembarkFunc, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("Cockpit: Force Eject Executed on %s"), *TargetSpaceship->GetName());
    }
    else
    {
        // 함수가 없다면 경고 출력
        UE_LOG(LogTemp, Error, TEXT("Cockpit: Target %s does NOT have 'DisembarkCharacter' function!"), *TargetSpaceship->GetName());
    }
}

void APSJ_ShipCockpit::AttemptBoarding(APSJ_Character* RequestingChar)
{
    // [1] 고장 상태에선 탑승 못 하게 거부함 (이 부분 먼저 체크 해야 안전함)
    if (bIsMalfunctioning)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Cockpit] System Error! Repair required before boarding."));
        return;
    }

    if (!RequestingChar) return;
    if (!TargetSpaceship)
    {
        UE_LOG(LogTemp, Error, TEXT("Cockpit: TargetSpaceship is MISSING! Set it in Editor Details."));
        return;
    }

    // 터렛인 경우
    if (ATurretBase_GT* TargetTurret = Cast<ATurretBase_GT>(TargetSpaceship))
    {
        // 캐릭터에게 "터렛 탑승 요청"를 전송 (서버로 처리하려고 Server RPC 사용)
        RequestingChar->Server_RequestTurretBoarding(TargetTurret, this);
    }
    // 우주선인 경우
    else if (APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship))
    {
        Spaceship->LinkedCockpit = this;
        // 캐릭터에게 "이 우주선 탑승"를 요청
        RequestingChar->Server_RequestBoarding(Spaceship);
    }
}


void APSJ_ShipCockpit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 서버에서만, 고장 상태일 때만 타이머 차감시킴.
    if (HasAuthority() && bIsMalfunctioning)
    {
        // === [수리속도 계산 공식 설명] ===
        // 기본 속도 감소: 1.0 (혼자 1배 속도임)
        // 추가 보너스: (RepairSpeedRate - 1.0)
        // 최종 감소 속도 = 1.0 + (수리인원수 * 추가 보너스)

        float BonusRatePerPerson = FMath::Max(1.0f, RepairSpeedRate) - 1.0f;
        float TotalSpeed = 1.0f + (RepairingCharacters.Num() * BonusRatePerPerson);

        // 타이머 차감
        CurrentMalfunctionTimer -= (DeltaTime * TotalSpeed);

        // 디버깅 로그 (너무 자주 나오면 주석처리)
        /*
        if (RepairingCharacters.Num() > 0)
        {
            UE_LOG(LogTemp, Log, TEXT("Repairing... Users: %d | Speed: x%.2f | TimeLeft: %.2f"),
                RepairingCharacters.Num(), TotalSpeed, CurrentMalfunctionTimer);
        }
        */

        // 수리 완료 시
        if (CurrentMalfunctionTimer <= 0.0f)
        {
            bIsMalfunctioning = false;
            CurrentMalfunctionTimer = 0.0f;
            RepairingCharacters.Empty();

            OnRep_IsMalfunctioning(); // 효과 재생
            UE_LOG(LogTemp, Log, TEXT("[Cockpit] REPAIR COMPLETE! System Online."));
        }
    }


    // [에디터에서 디버그용 텍스트 표시]
    if (GetWorld()->IsNetMode(NM_Client) || GetWorld()->IsPlayInEditor())
    {
        FVector ActorLoc = GetActorLocation();

        // 1. [링크] 대상 연결 표시 (위치: 원점)
        if (TargetSpaceship)
        {
            DrawDebugString(GetWorld(), ActorLoc, TEXT("Link OK"), nullptr, FColor::Green, 0.0f);
        }
        else
        {
            DrawDebugString(GetWorld(), ActorLoc, TEXT("Link NULL"), nullptr, FColor::White, 0.0f);
        }

        // 2. [상태] 고장 상태 표시 (위치: Z축으로 50cm 위)
        FVector StatusLoc = ActorLoc + FVector(0, 0, 50.0f); // 원점 위쪽에서 띄워서 표시

        if (bIsMalfunctioning)
        {
            // 고장중일 때: 빨간색으로 남은 시간 표시
            FString StatusMsg = FString::Printf(TEXT("MALFUNCTION! (Time: %.1f)"), CurrentMalfunctionTimer);
            DrawDebugString(GetWorld(), StatusLoc, StatusMsg, nullptr, FColor::Red, 0.0f);
        }
        else
        {
            // 정상일 때: 청록색으로 Normal 표시
            DrawDebugString(GetWorld(), StatusLoc, TEXT("STATUS: NORMAL"), nullptr, FColor::Cyan, 0.0f);
        }

        // 3. [수리] 수리 중인 인원 표시 (위치: Z축으로 80cm 위)
        if (RepairingCharacters.Num() > 0)
        {
            FVector RepairLoc = ActorLoc + FVector(0, 0, 80.0f);
            FString RepairMsg = FString::Printf(TEXT("Repairing... (%d People)"), RepairingCharacters.Num());
            DrawDebugString(GetWorld(), RepairLoc, RepairMsg, nullptr, FColor::Yellow, 0.0f);
        }
    }

}

// [5] 수리 인원 관리 (Character에서 호출)
void APSJ_ShipCockpit::AddRepairer(APSJ_Character* Mechanic)
{
    if (Mechanic && !RepairingCharacters.Contains(Mechanic))
    {
        RepairingCharacters.Add(Mechanic);
    }
}

void APSJ_ShipCockpit::RemoveRepairer(APSJ_Character* Mechanic)
{
    if (Mechanic)
    {
        RepairingCharacters.Remove(Mechanic);
    }
}

// [6] RepNotify (클라이언트 시각 효과)
void APSJ_ShipCockpit::OnRep_IsMalfunctioning()
{
    if (bIsMalfunctioning)
    {
        // 고장: 빨갛게 깜빡임, 경고음 재생 등등
        UE_LOG(LogTemp, Warning, TEXT("[Client] Cockpit looks broken!"));
    }
    else
    {
        // 정상: 초록색 불빛, 정상 사운드 재생
        UE_LOG(LogTemp, Log, TEXT("[Client] Cockpit looks fixed!"));
    }
}