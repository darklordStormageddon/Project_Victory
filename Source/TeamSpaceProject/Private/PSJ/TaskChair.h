#pragma once

#include "CoreMinimal.h"
#include "JHS/Interact/InteractableActorBase.h"
#include "YSH/SolarWindManager.h"
#include "TaskChair.generated.h"

class ATaskPawnBase;
class UUIBase;
class APSJ_Character; 

UCLASS()
class ATaskChair : public AInteractableActorBase
{
	GENERATED_BODY()

public:
	ATaskChair(); 

private:


protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI) override;

	UPROPERTY(EditAnywhere, Replicated, Category = "Link")
	TObjectPtr<ATaskPawnBase> TargetTaskPawn = nullptr;

public:

	UFUNCTION(BlueprintCallable, Category = "Link")
	void SetTargetTaskPawn(ATaskPawnBase* NewTaskPawn);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_IsOccupied, Category = "State")
	bool bIsOccupied = false;

	UFUNCTION()
	void OnRep_IsOccupied();


	virtual void OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI) override;

	ATaskPawnBase* GetTargetTaskPawn() const { return TargetTaskPawn; }
	virtual void Tick(float DeltaTime) override;

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
	UPROPERTY(Replicated)
	TArray<APSJ_Character*> RepairingCharacters;

	void StartMalfunction();
	void AddRepairer(APSJ_Character* Mechanic);
	void RemoveRepairer(APSJ_Character* Mechanic);

	UFUNCTION()
	void OnRep_IsMalfunctioning();

	void ReceiveForceEjectRequest();
};