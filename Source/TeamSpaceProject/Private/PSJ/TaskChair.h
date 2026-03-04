#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"
#include "YSH/SolarWindManager.h" // 추가: 태양풍 매니저
#include "TaskChair.generated.h"

class ATaskPawnBase;
class UUIBase;
class APSJ_Character; // 전방 선언 추가

UCLASS()
class ATaskChair : public AInteractableActorBase
{
	GENERATED_BODY()

public:
	ATaskChair(); // 생성자 추가 (TickGroup 설정을 위해 필요)

private:


protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI) override;

	UPROPERTY(EditAnywhere, Replicated, Category = "Link")
	TObjectPtr<ATaskPawnBase> TargetTaskPawn = nullptr;

public:

	// 블루프린트에서 스폰한 포탑을 의자에 연결할 수 있도록 함수를 열어줍니다.
	UFUNCTION(BlueprintCallable, Category = "Link")
	void SetTargetTaskPawn(ATaskPawnBase* NewTaskPawn);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsOccupied, Category = "State")
	bool bIsOccupied = false;

	UFUNCTION()
	void OnRep_IsOccupied();


	virtual void OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI) override;

	ATaskPawnBase* GetTargetTaskPawn() const { return TargetTaskPawn; }
	virtual void Tick(float DeltaTime) override;

	// =========================================================
	// [이식됨] 태양풍 기능 고장 (Malfunction) 시스템
	// =========================================================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsMalfunctioning, Category = "Malfunction")
	bool bIsMalfunctioning = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FVector SeatDisembarkOffset = FVector(0.0f, 0.0f, 0.0f);

	UFUNCTION()
	void HandleSolarWindEvent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Malfunction")
	float MalfunctionProbability = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Malfunction")
	float MalfunctionDuration = 20.0f;

	float CurrentMalfunctionTimer = 0.0f;
	TArray<APSJ_Character*> RepairingCharacters;

	void StartMalfunction();
	void AddRepairer(APSJ_Character* Mechanic);
	void RemoveRepairer(APSJ_Character* Mechanic);

	UFUNCTION()
	void OnRep_IsMalfunctioning();

	// [이식됨] 외부에서 강제 하차 요청이 들어왔을 때 실행할 함수
	void ReceiveForceEjectRequest();
};