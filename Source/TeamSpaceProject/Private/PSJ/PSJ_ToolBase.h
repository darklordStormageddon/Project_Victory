#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PSJ_ToolBase.generated.h"

class UStaticMeshComponent;
class UArrowComponent;
class UAnimMontage;
class UParticleSystem;
class APSJ_Character;
class UNiagaraSystem;
class UNiagaraComponent;


UCLASS()
class TEAMSPACEPROJECT_API APSJ_ToolBase : public AActor
{
	GENERATED_BODY()

public:
	APSJ_ToolBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool")
	UStaticMeshComponent* ToolMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool")
	UArrowComponent* TraceMuzzle;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tool|State")
	bool bIsOnCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Settings")
	float ForceEjectCooldownTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Settings")
	float NormalRepairSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Settings")
	float CooldownRepairSpeed = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	UParticleSystem* Effect_EjectSuccess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	UParticleSystem* Effect_EjectFail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Animation")
	UAnimMontage* Montage_ForceEject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Animation")
	UAnimMontage* Montage_Repair;

	float GetCurrentRepairSpeed() const;

	void StartCooldownTimer();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayEjectEffect(bool bSuccess, APSJ_Character* InstigatorChar);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetToolHidden(bool bHide);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	UNiagaraSystem* Niagara_RepairNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	UNiagaraSystem* Niagara_RepairCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	FVector RepairEffectLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	FRotator RepairEffectRotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	FVector RepairEffectScale = FVector(1.0f, 1.0f, 1.0f);

	UPROPERTY()
	UNiagaraComponent* ActiveRepairNiagara;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetRepairEffectActive(bool bActive);

protected:
	FTimerHandle CooldownTimerHandle;
	void ResetCooldown();
};