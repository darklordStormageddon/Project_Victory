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
		GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &APSJ_ToolBase::ResetCooldown, ForceEjectCooldownTime, false);
	}
}

void APSJ_ToolBase::ResetCooldown()
{
	bIsOnCooldown = false;
}

void APSJ_ToolBase::Multicast_PlayEjectEffect_Implementation(bool bSuccess, APSJ_Character* InstigatorChar)
{
	// 1. 성공/실패에 따른 파티클 재생
	UParticleSystem* EffectToPlay = bSuccess ? Effect_EjectSuccess : Effect_EjectFail;
	if (EffectToPlay)
	{
		// 툴의 앞부분(Muzzle)에서 이펙트 발생
		UGameplayStatics::SpawnEmitterAttached(EffectToPlay, TraceMuzzle);
	}

	// 2. 캐릭터 애니메이션 재생 (옵션)
	if (InstigatorChar && Montage_ForceEject)
	{
		InstigatorChar->PlayAnimMontage(Montage_ForceEject);
	}
}