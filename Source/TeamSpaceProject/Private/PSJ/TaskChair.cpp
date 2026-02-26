// Fill out your copyright notice in the Description page of Project Settings.


#include "PSJ/TaskChair.h"
#include "JHS/UI/UIBase.h"
#include "PSJ/TaskPawnBase.h"
#include "PSJ_Spaceship.h"
#include "PSJ_ToolBase.h"
#include "Net/UnrealNetwork.h"
#include "PSJ_Character.h" 
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h" // APawn 사용을 위해 필요
#include "GameFramework/Controller.h" // Controller 체크를 위해 필요
#include "GameFramework/PlayerController.h"

ATaskChair::ATaskChair()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = true;
}

void ATaskChair::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AActor* ManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASolarWindManager::StaticClass());
		if (ASolarWindManager* WindManager = Cast<ASolarWindManager>(ManagerActor))
		{
			WindManager->OnSolarWindImpact.AddDynamic(this, &ATaskChair::HandleSolarWindEvent);
		}
	}
}

void ATaskChair::SetTargetTaskPawn(ATaskPawnBase* NewTaskPawn)
{
    TargetTaskPawn = NewTaskPawn;
}

void ATaskChair::HandleSolarWindEvent()
{
	// 서버인지 한번 더 체크 (안전장치)
	if (!HasAuthority()) return;

	// 이미 고장난 상태면 확률 계산 없이 패스하거나 타이머 갱신 (선택사항)
	if (bIsMalfunctioning) return;

	// 1. 확률 계산 (주사위 굴리기)
	float DiceRoll = FMath::FRand(); // 0.0 ~ 1.0 랜덤

	if (DiceRoll <= MalfunctionProbability)
	{
		// 당첨! 고장 로직 실행
		UE_LOG(LogTemp, Warning, TEXT("[Cockpit] Hit by Solar Wind! (Roll: %.2f <= Prob: %.2f)"), DiceRoll, MalfunctionProbability);
		StartMalfunction();
	}
	else
	{
		// 회피 성공
		UE_LOG(LogTemp, Log, TEXT("[Cockpit] Survived Solar Wind. (Roll: %.2f > Prob: %.2f)"), DiceRoll, MalfunctionProbability);
	}
}

// [3] 고장 발생 (SolarWindManager가 호출) -- 강제하차 기존함수활용은 맞는데 TaskChair에 맞게 수정해야함
void ATaskChair::StartMalfunction()
{
	if (!HasAuthority()) return; // 서버만 실행

	// 이미 고장난 상태면 타이머만 리셋 (또는 무시 가능)
	if (bIsMalfunctioning)
	{
		CurrentMalfunctionTimer = MalfunctionDuration;
		return;
	}

	// 1. 탑승자 강제 하차 (기존 함수 활용)
	ReceiveForceEjectRequest();

	// 2. 상태 변경
	bIsMalfunctioning = true;
	CurrentMalfunctionTimer = MalfunctionDuration;
	RepairingCharacters.Empty(); // 수리 인원 초기화

	// 3. 상태 갱신 (OnRep 호출됨)
	OnRep_IsMalfunctioning();

	UE_LOG(LogTemp, Error, TEXT("[Cockpit] MALFUNCTION STARTED! Timer: %.1f"), MalfunctionDuration);
}

void ATaskChair::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATaskChair, TargetTaskPawn);
	DOREPLIFETIME(ATaskChair, bIsMalfunctioning); // 추가
}

void ATaskChair::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{
	// 1. 고장 났을 때는 탑승 불가
	if (bIsMalfunctioning)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TaskChair] System Error! Repair required before boarding."));
		return;
	}

	Super::OnInteractEnter(CallerPlayerId, OpenedUI);

	if (TargetTaskPawn == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("ATaskChair: TargetTaskPawn is nullptr"));
		return;
	}

	APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn);

	if (!MyChar) return;

	MyChar->Server_RequestBoarding(TargetTaskPawn);
}

void ATaskChair::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
	Super::OnInteractExit(CallerPlayerId, ClosedUI);
}

void ATaskChair::ReceiveForceEjectRequest()
{
    if (!TargetTaskPawn || TargetTaskPawn->GetController() == nullptr) return;

    TargetTaskPawn->DisembarkCharacter();
}

void ATaskChair::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority() && bIsMalfunctioning)
    {
        float TotalSpeed = 1.0f;

        for (APSJ_Character* Mechanic : RepairingCharacters)
        {
            if (Mechanic && Mechanic->EquippedTool)
            {
                TotalSpeed += Mechanic->EquippedTool->GetCurrentRepairSpeed();
            }
            else
            {
                TotalSpeed += 0.5f;
            }
        }


        CurrentMalfunctionTimer -= (DeltaTime * TotalSpeed);


        if (CurrentMalfunctionTimer <= 0.0f)
        {
            bIsMalfunctioning = false;
            CurrentMalfunctionTimer = 0.0f;
            RepairingCharacters.Empty();

            OnRep_IsMalfunctioning(); 
            UE_LOG(LogTemp, Log, TEXT("[Cockpit] REPAIR COMPLETE! System Online."));
        }
    }


    if (GetWorld()->IsNetMode(NM_Client) || GetWorld()->IsPlayInEditor())
    {
        FVector ActorLoc = GetActorLocation();

        if (TargetTaskPawn)
        {
            DrawDebugString(GetWorld(), ActorLoc, TEXT("Link OK"), nullptr, FColor::Green, 0.0f);
        }
        else
        {
            DrawDebugString(GetWorld(), ActorLoc, TEXT("Link NULL"), nullptr, FColor::White, 0.0f);
        }


        FVector StatusLoc = ActorLoc + FVector(0, 0, 50.0f); 

        if (bIsMalfunctioning)
        {

            FString StatusMsg = FString::Printf(TEXT("MALFUNCTION! (Time: %.1f)"), CurrentMalfunctionTimer);
            DrawDebugString(GetWorld(), StatusLoc, StatusMsg, nullptr, FColor::Red, 0.0f);
        }
        else
        {

            DrawDebugString(GetWorld(), StatusLoc, TEXT("STATUS: NORMAL"), nullptr, FColor::Cyan, 0.0f);
        }


        if (RepairingCharacters.Num() > 0)
        {
            FVector RepairLoc = ActorLoc + FVector(0, 0, 80.0f);
            FString RepairMsg = FString::Printf(TEXT("Repairing... (%d People)"), RepairingCharacters.Num());
            DrawDebugString(GetWorld(), RepairLoc, RepairMsg, nullptr, FColor::Yellow, 0.0f);
        }
    }

}


void ATaskChair::AddRepairer(APSJ_Character* Mechanic)
{
    if (Mechanic && !RepairingCharacters.Contains(Mechanic))
    {
        RepairingCharacters.Add(Mechanic);
    }
}

void ATaskChair::RemoveRepairer(APSJ_Character* Mechanic)
{
    if (Mechanic)
    {
        RepairingCharacters.Remove(Mechanic);
    }
}

// [6] RepNotify (클라이언트 효과 처리)
void ATaskChair::OnRep_IsMalfunctioning()
{
    if (bIsMalfunctioning)
    {
        // 예: 스파크 파티클 켜기, 고장음 루프 재생
        UE_LOG(LogTemp, Warning, TEXT("[Client] Cockpit looks broken!"));
    }
    else
    {
        // 예: 파티클 끄기, 정상 상태 복구
        UE_LOG(LogTemp, Log, TEXT("[Client] Cockpit looks fixed!"));
    }
}