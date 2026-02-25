//
//#include "PSJ_ShipCockpit.h"
//#include "Kismet/GameplayStatics.h" // 매니저 찾기용
//#include "PSJ_Character.h" 
//#include "PSJ_Spaceship.h"
//#include "PSJ_ToolBase.h"
//#include "YSH/TurretBase_GT.h"
//#include "Net/UnrealNetwork.h" // [필수] 이 헤더가 맨 위에 있어야 합니다
//#include "GameFramework/Pawn.h" // APawn 사용을 위해 필요
//#include "GameFramework/Controller.h" // Controller 체크를 위해 필요
//#include "GameFramework/PlayerController.h"
//#include "JHS/UI/UIBase.h"
//
//// [1] 생성자 구현 (이 부분이 없어서 아까 에러가 난 것입니다)
//APSJ_ShipCockpit::APSJ_ShipCockpit()
//{
//    // 틱 활성화
//    PrimaryActorTick.bCanEverTick = true;
//
//    // [핵심 해결책] 
//    // 물리(Physics) 이동이 끝난 '뒤'에 틱을 실행해라!
//    // -> 이 설정 덕분에 BP에서 그리는 디버그 라인도 밀리지 않고 딱 붙어 나옵니다.
//    PrimaryActorTick.TickGroup = TG_PostPhysics;
//
//    // [!!! 핵심 누락 수정 !!!] 
//    // 이 줄이 없으면 변수 동기화(Replicated)가 아예 작동하지 않습니다.
//    bReplicates = true;
//}
//
//void APSJ_ShipCockpit::BeginPlay()
//{
//    Super::BeginPlay();
//
//    // [중요] 서버에서만 이벤트를 처리하면 됩니다.
//    if (HasAuthority())
//    {
//        // 1. 월드에 있는 태양풍 매니저를 찾습니다.
//        // (보통 매니저는 1개만 존재하므로 GetActorOfClass 사용)
//        AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASolarWindManager::StaticClass());
//
//        if (ASolarWindManager* WindManager = Cast<ASolarWindManager>(ManagerActor))
//        {
//            // 2. 이벤트에 내 함수를 등록(Bind)합니다.
//            // "매니저님, 태양풍 터지면 저한테도(HandleSolarWindEvent) 알려주세요"
//            WindManager->OnSolarWindImpact.AddDynamic(this, &APSJ_ShipCockpit::HandleSolarWindEvent);
//
//            UE_LOG(LogTemp, Log, TEXT("[Cockpit] Successfully bound to SolarWindManager."));
//        }
//    }
//}
//
//// [신규] 이벤트 수신 함수 (Delegate에 의해 호출됨)
//void APSJ_ShipCockpit::HandleSolarWindEvent()
//{
//    // 서버인지 한번 더 체크 (안전장치)
//    if (!HasAuthority()) return;
//
//    // 이미 고장난 상태면 확률 계산 없이 패스하거나 타이머 갱신 (선택사항)
//    if (bIsMalfunctioning) return;
//
//    // 1. 확률 계산 (주사위 굴리기)
//    float DiceRoll = FMath::FRand(); // 0.0 ~ 1.0 랜덤
//
//    if (DiceRoll <= MalfunctionProbability)
//    {
//        // 당첨! 고장 로직 실행
//        UE_LOG(LogTemp, Warning, TEXT("[Cockpit] Hit by Solar Wind! (Roll: %.2f <= Prob: %.2f)"), DiceRoll, MalfunctionProbability);
//        StartMalfunction();
//    }
//    else
//    {
//        // 회피 성공
//        UE_LOG(LogTemp, Log, TEXT("[Cockpit] Survived Solar Wind. (Roll: %.2f > Prob: %.2f)"), DiceRoll, MalfunctionProbability);
//    }
//}
//
//// [신규 추가] 변수 동기화 규칙 설정
//void APSJ_ShipCockpit::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//    // TargetSpaceship 변수를 서버 -> 클라이언트로 복제(Replication)합니다.
//    DOREPLIFETIME(APSJ_ShipCockpit, TargetSpaceship);
//
//    // [신규] 고장 상태 동기화 (이게 없으면 클라에서 고장난 줄 모름)
//    DOREPLIFETIME(APSJ_ShipCockpit, bIsMalfunctioning);
//}
//
//void APSJ_ShipCockpit::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
//{
//
//    // [신규] 고장 상태 체크
//    if (bIsMalfunctioning)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("[Cockpit] System Error! Repair required before boarding."));
//        // 여기에 "수리가 필요합니다" 같은 팝업이나 사운드를 재생할 수 있습니다.
//        return;
//    }
//
//    Super::OnInteractEnter(CallerPlayerId, OpenedUI); // 부모의 기본 로직 실행
//
//    if (TargetSpaceship) // 변수명은 TargetSpaceship이지만 실제로는 APawn* 타입
//    {
//        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
//        APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn);
//
//        if (!MyChar) return;
//
//        // [분기 1] 대상이 터렛(TurretBase_GT)인 경우
//        if (ATurretBase_GT* TargetTurret = Cast<ATurretBase_GT>(TargetSpaceship))
//        {
//            // 터렛용 탑승 요청 호출
//            MyChar->Server_RequestTurretBoarding(TargetTurret, this);
//            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting boarding to TURRET: %s"), *TargetTurret->GetName());
//        }
//        // [분기 2] 대상이 우주선(Spaceship)인 경우
//        else if (APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship))
//        {
//            // 우주선에 조종석 연결 정보 전달
//            Spaceship->LinkedCockpit = this;
//            MyChar->Server_RequestBoarding(Spaceship);
//            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting boarding to SPACESHIP: %s"), *Spaceship->GetName());
//        }
//    }
//}
//
//// [3] 고장 발생 (SolarWindManager가 호출)
//void APSJ_ShipCockpit::StartMalfunction()
//{
//    if (!HasAuthority()) return; // 서버만 실행
//
//    // 이미 고장난 상태면 타이머만 리셋 (또는 무시 가능)
//    if (bIsMalfunctioning)
//    {
//        CurrentMalfunctionTimer = MalfunctionDuration;
//        return;
//    }
//
//    // 1. 탑승자 강제 하차 (기존 함수 활용)
//    ReceiveForceEjectRequest();
//
//    // 2. 상태 변경
//    bIsMalfunctioning = true;
//    CurrentMalfunctionTimer = MalfunctionDuration;
//    RepairingCharacters.Empty(); // 수리 인원 초기화
//
//    // 3. 상태 갱신 (OnRep 호출됨)
//    OnRep_IsMalfunctioning();
//
//    UE_LOG(LogTemp, Error, TEXT("[Cockpit] MALFUNCTION STARTED! Timer: %.1f"), MalfunctionDuration);
//}
//
//
//void APSJ_ShipCockpit::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
//{
//    // 1. TargetSpaceship이 유효한지 확인
//    if (TargetSpaceship)
//    {
//        // 2. APawn 타입을 APSJ_Spaceship 타입으로 형변환
//        APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship);
//
//        // 3. 형변환에 성공했고, 현재 조종사가 있다면 종료(return) 처리
//        if (Spaceship && Spaceship->GetCurrentPilot())
//        {
//            return;
//        }
//    }
//
//    // 4. 조종사가 없거나 형변환에 실패한 경우 부모 로직 실행
//    Super::OnInteractExit(CallerPlayerId, OpenedUI);
//}
//
//void APSJ_ShipCockpit::SetTargetPawn(TObjectPtr<APawn> TargetPawn)
//{
//    TargetSpaceship = TargetPawn;
//}
//
//void APSJ_ShipCockpit::ReceiveForceEjectRequest()
//{
//    // 1. 연결된 대상(우주선, 터렛 등)이 아예 세팅 안 된 경우 -> 무시
//    if (!TargetSpaceship) return;
//
//    // [핵심 안전장치] 대상 Pawn에 현재 빙의(Possess)한 컨트롤러(플레이어)가 있는가?
//    // Controller가 nullptr라면, 타고 있는 사람이 없다는 뜻입니다.
//    if (TargetSpaceship->GetController() == nullptr)
//    {
//        // 사람이 없으므로 하차 로직을 실행하지 않고 그냥 나갑니다.
//        // 역으로 내가 탑승하는 로직도 없으므로 안전합니다.
//        // UE_LOG(LogTemp, Log, TEXT("Cockpit: Seat is empty. No one to eject."));
//        return;
//    }
//
//    // --- 사람이 있을 때만 아래 로직이 실행됩니다 ---
//
//    // 2. 리플렉션을 이용해 연결된 Pawn의 'DisembarkCharacter' 함수 찾기
//    static const FName DisembarkFuncName(TEXT("DisembarkCharacter"));
//    UFunction* DisembarkFunc = TargetSpaceship->FindFunction(DisembarkFuncName);
//
//    if (DisembarkFunc)
//    {
//        // 함수가 있으면 실행 (강제 하차)
//        TargetSpaceship->ProcessEvent(DisembarkFunc, nullptr);
//        UE_LOG(LogTemp, Warning, TEXT("Cockpit: Force Eject Executed on %s"), *TargetSpaceship->GetName());
//    }
//    else
//    {
//        // 함수가 없으면 로그 출력
//        UE_LOG(LogTemp, Error, TEXT("Cockpit: Target %s does NOT have 'DisembarkCharacter' function!"), *TargetSpaceship->GetName());
//    }
//}
//
//void APSJ_ShipCockpit::AttemptBoarding(APSJ_Character* RequestingChar)
//{
//    // [1] 고장 났으면 탑승 시도 자체를 차단 (이 코드가 없어서 뚫렸던 것입니다)
//    if (bIsMalfunctioning)
//    {
//        UE_LOG(LogTemp, Warning, TEXT("[Cockpit] System Error! Repair required before boarding."));
//        return;
//    }
//
//    if (!RequestingChar) return;
//    if (!TargetSpaceship)
//    {
//        UE_LOG(LogTemp, Error, TEXT("Cockpit: TargetSpaceship is MISSING! Set it in Editor Details."));
//        return;
//    }
//
//    // 터렛인 경우
//    if (ATurretBase_GT* TargetTurret = Cast<ATurretBase_GT>(TargetSpaceship))
//    {
//        // 서버에게 "나 터렛 탈래"라고 요청 (캐릭터 내부에서 Server RPC 호출됨)
//        RequestingChar->Server_RequestTurretBoarding(TargetTurret, this);
//    }
//    // 우주선인 경우
//    else if (APSJ_Spaceship* Spaceship = Cast<APSJ_Spaceship>(TargetSpaceship))
//    {
//        Spaceship->LinkedCockpit = this;
//        // 서버에게 "나 우주선 탈래"라고 요청
//        RequestingChar->Server_RequestBoarding(Spaceship);
//    }
//}
//
//
//void APSJ_ShipCockpit::Tick(float DeltaTime)
//{
//    Super::Tick(DeltaTime);
//
//    // 서버이고, 고장난 상태일 때만 타이머가 돌아갑니다.
//    if (HasAuthority() && bIsMalfunctioning)
//    {
//        // [수정] 수리 가속도 이원화 반영
//              // 기본적으로 시간이 흐르므로 1.0f 베이스
//        float TotalSpeed = 1.0f;
//
//        for (APSJ_Character* Mechanic : RepairingCharacters)
//        {
//            if (Mechanic && Mechanic->EquippedTool)
//            {
//                // 캐릭터의 툴 상태에 따라 Normal(1.5) 혹은 Cooldown(0.5) 속도 합산
//                TotalSpeed += Mechanic->EquippedTool->GetCurrentRepairSpeed();
//            }
//            else
//            {
//                // 혹시 툴을 못 든 상태라면 최소한의 속도만 보장
//                TotalSpeed += 0.5f;
//            }
//        }
//
//
//        // 시간 감소
//        CurrentMalfunctionTimer -= (DeltaTime * TotalSpeed);
//
//        // 디버깅용 로그 (개발 중에만 켜두세요)
//        /*
//        if (RepairingCharacters.Num() > 0)
//        {
//            UE_LOG(LogTemp, Log, TEXT("Repairing... Users: %d | Speed: x%.2f | TimeLeft: %.2f"),
//                RepairingCharacters.Num(), TotalSpeed, CurrentMalfunctionTimer);
//        }
//        */
//
//        // 수리 완료 체크
//        if (CurrentMalfunctionTimer <= 0.0f)
//        {
//            bIsMalfunctioning = false;
//            CurrentMalfunctionTimer = 0.0f;
//            RepairingCharacters.Empty();
//
//            OnRep_IsMalfunctioning(); // 클라에 알림
//            UE_LOG(LogTemp, Log, TEXT("[Cockpit] REPAIR COMPLETE! System Online."));
//        }
//    }
//
//
//    // [클라이언트 디버깅용 텍스트 표시]
//    if (GetWorld()->IsNetMode(NM_Client) || GetWorld()->IsPlayInEditor())
//    {
//        FVector ActorLoc = GetActorLocation();
//
//        // 1. [기존] 연결 상태 표시 (위치: 기본)
//        if (TargetSpaceship)
//        {
//            DrawDebugString(GetWorld(), ActorLoc, TEXT("Link OK"), nullptr, FColor::Green, 0.0f);
//        }
//        else
//        {
//            DrawDebugString(GetWorld(), ActorLoc, TEXT("Link NULL"), nullptr, FColor::White, 0.0f);
//        }
//
//        // 2. [신규] 고장 상태 표시 (위치: Z축으로 50cm 위)
//        FVector StatusLoc = ActorLoc + FVector(0, 0, 50.0f); // 글자 겹치지 않게 위로 띄움
//
//        if (bIsMalfunctioning)
//        {
//            // 고장났을 때: 빨간색으로 남은 시간 표시
//            FString StatusMsg = FString::Printf(TEXT("MALFUNCTION! (Time: %.1f)"), CurrentMalfunctionTimer);
//            DrawDebugString(GetWorld(), StatusLoc, StatusMsg, nullptr, FColor::Red, 0.0f);
//        }
//        else
//        {
//            // 정상일 때: 파란색으로 Normal 표시
//            DrawDebugString(GetWorld(), StatusLoc, TEXT("STATUS: NORMAL"), nullptr, FColor::Cyan, 0.0f);
//        }
//
//        // 3. [신규] 수리 중인 인원 표시 (위치: Z축으로 80cm 위)
//        if (RepairingCharacters.Num() > 0)
//        {
//            FVector RepairLoc = ActorLoc + FVector(0, 0, 80.0f);
//            FString RepairMsg = FString::Printf(TEXT("Repairing... (%d People)"), RepairingCharacters.Num());
//            DrawDebugString(GetWorld(), RepairLoc, RepairMsg, nullptr, FColor::Yellow, 0.0f);
//        }
//    }
//
//}
//
//// [5] 수리 인원 관리 (Character에서 호출됨)
//void APSJ_ShipCockpit::AddRepairer(APSJ_Character* Mechanic)
//{
//    if (Mechanic && !RepairingCharacters.Contains(Mechanic))
//    {
//        RepairingCharacters.Add(Mechanic);
//    }
//}
//
//void APSJ_ShipCockpit::RemoveRepairer(APSJ_Character* Mechanic)
//{
//    if (Mechanic)
//    {
//        RepairingCharacters.Remove(Mechanic);
//    }
//}
//
//// [6] RepNotify (클라이언트 효과 처리)
//void APSJ_ShipCockpit::OnRep_IsMalfunctioning()
//{
//    if (bIsMalfunctioning)
//    {
//        // 예: 스파크 파티클 켜기, 고장음 루프 재생
//        UE_LOG(LogTemp, Warning, TEXT("[Client] Cockpit looks broken!"));
//    }
//    else
//    {
//        // 예: 파티클 끄기, 정상 상태 복구
//        UE_LOG(LogTemp, Log, TEXT("[Client] Cockpit looks fixed!"));
//    }
//}