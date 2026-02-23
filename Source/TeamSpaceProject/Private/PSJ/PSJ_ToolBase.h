#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PSJ_ToolBase.generated.h"

class UStaticMeshComponent;
class UArrowComponent;
class UAnimMontage;
class UParticleSystem;
class APSJ_Character;

UCLASS()
class TEAMSPACEPROJECT_API APSJ_ToolBase : public AActor
{
	GENERATED_BODY()

public:
	APSJ_ToolBase();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 모델링 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool")
	UStaticMeshComponent* ToolMesh;

	// [핵심] 트레이스가 나갈 방향과 위치를 지정해줄 화살표 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool")
	UArrowComponent* TraceMuzzle;

	// --- 쿨다운 및 속도 변수 (에디터 노출) ---
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tool|State")
	bool bIsOnCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Settings")
	float ForceEjectCooldownTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Settings")
	float NormalRepairSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Settings")
	float CooldownRepairSpeed = 0.5f;

	// --- 이펙트 및 애니메이션 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	UParticleSystem* Effect_EjectSuccess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Effects")
	UParticleSystem* Effect_EjectFail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Animation")
	UAnimMontage* Montage_ForceEject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tool|Animation")
	UAnimMontage* Montage_Repair;

	// 현재 상태에 따른 수리 속도 반환 함수
	float GetCurrentRepairSpeed() const;

	// 서버에서 쿨다운 타이머 시작
	void StartCooldownTimer();

	// 모두에게 이펙트/애니메이션 재생 지시
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayEjectEffect(bool bSuccess, APSJ_Character* InstigatorChar);

protected:
	FTimerHandle CooldownTimerHandle;
	void ResetCooldown();
};