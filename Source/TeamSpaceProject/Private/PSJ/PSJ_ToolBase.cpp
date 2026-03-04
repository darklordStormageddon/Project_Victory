#include "PSJ_ToolBase.h"
#include "PSJ_Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

APSJ_ToolBase::APSJ_ToolBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	SetRootComponent(ToolMesh);

	TraceMuzzle = CreateDefaultSubobject<UArrowComponent>(TEXT("TraceMuzzle"));
	TraceMuzzle->SetupAttachment(ToolMesh);
}

void APSJ_ToolBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSJ_ToolBase, bIsOnCooldown);
}

float APSJ_ToolBase::GetCurrentRepairSpeed() const
{
	// 쿨다운 중이면 페널티 속도, 아니면 정상 속도 반환
	return bIsOnCooldown ? CooldownRepairSpeed : NormalRepairSpeed;
}

void APSJ_ToolBase::StartCooldownTimer()
{
	if (HasAuthority())
	{
		bIsOnCooldown = true;
		// 기존에 있던 UpdateToolVisibility() 호출 제거
		GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &APSJ_ToolBase::ResetCooldown, ForceEjectCooldownTime, false);
	}
}

void APSJ_ToolBase::ResetCooldown()
{
	bIsOnCooldown = false;

	// [추가] 쿨다운 타이머가 끝났을 때 도구가 숨겨져 있는 상태라면 강제로 보이게 원상복구 시켜줍니다.
	// 이 함수는 서버(HasAuthority)에서 실행되므로 안전하게 멀티캐스트 호출이 가능합니다.
	Multicast_SetToolHidden(false);
}

void APSJ_ToolBase::Multicast_PlayEjectEffect_Implementation(bool bSuccess, APSJ_Character* InstigatorChar)
{
	// 1. 성공/실패 파티클 재생
	UParticleSystem* EffectToPlay = bSuccess ? Effect_EjectSuccess : Effect_EjectFail;
	if (EffectToPlay)
	{
		UGameplayStatics::SpawnEmitterAttached(EffectToPlay, TraceMuzzle);
	}

	// 2. 애니메이션 동기화
	if (InstigatorChar && Montage_ForceEject)
	{
		if (!InstigatorChar->IsLocallyControlled())
		{
			InstigatorChar->PlayAnimMontage(Montage_ForceEject);
		}
	}
}

// =========================================================================
// [추가] 멀티플레이어 환경에서 액터 자체를 확실하게 숨기는 로직
// =========================================================================
void APSJ_ToolBase::Multicast_SetToolHidden_Implementation(bool bHide)
{
	// 컴포넌트 단위의 SetVisibility 보다 액터 전체를 숨기는 것이 부착된 장비 동기화에 훨씬 안정적입니다.
	SetActorHiddenInGame(bHide);
}