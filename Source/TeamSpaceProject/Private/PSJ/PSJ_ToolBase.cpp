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

	Multicast_SetToolHidden(false);
}

void APSJ_ToolBase::Multicast_PlayEjectEffect_Implementation(bool bSuccess, APSJ_Character* InstigatorChar)
{
	UParticleSystem* EffectToPlay = bSuccess ? Effect_EjectSuccess : Effect_EjectFail;
	if (EffectToPlay)
	{
		UGameplayStatics::SpawnEmitterAttached(EffectToPlay, TraceMuzzle);
	}

	if (InstigatorChar && Montage_ForceEject)
	{
		if (!InstigatorChar->IsLocallyControlled())
		{
			InstigatorChar->PlayAnimMontage(Montage_ForceEject);
		}
	}
}


void APSJ_ToolBase::Multicast_SetToolHidden_Implementation(bool bHide)
{
	SetActorHiddenInGame(bHide);
}