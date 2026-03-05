#include "PSJ_ToolBase.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
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

	if (APSJ_Character* OwnerChar = Cast<APSJ_Character>(GetOwner()))
	{
		if (OwnerChar->bIsActivelyRepairing)
		{
			Multicast_SetRepairEffectActive(true); 
		}
	}
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

void APSJ_ToolBase::Multicast_SetRepairEffectActive_Implementation(bool bActive)
{
	// 1. ±âÁ¸ Àç»ý ÁßÀÎ ÀÌÆåÆ® ²ô±â
	if (ActiveRepairNiagara)
	{
		ActiveRepairNiagara->Deactivate();
		ActiveRepairNiagara->DestroyComponent();
		ActiveRepairNiagara = nullptr;
	}

	// 2. ÀÌÆåÆ® ÄÑ±â
	if (bActive)
	{
		UNiagaraSystem* SystemToPlay = bIsOnCooldown ? Niagara_RepairCooldown : Niagara_RepairNormal;

		if (SystemToPlay)
		{
			ActiveRepairNiagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
				SystemToPlay,
				TraceMuzzle,        
				NAME_None,
				RepairEffectLocationOffset, 
				RepairEffectRotationOffset, 
				EAttachLocation::KeepRelativeOffset,
				true
			);
			if (ActiveRepairNiagara)
			{
				ActiveRepairNiagara->SetRelativeScale3D(RepairEffectScale);
			}
		}
	}
}