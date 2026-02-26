// [PSJ_Character.h]

#pragma once
#define ECC_Spaceship_Floor ECC_GameTraceChannel5
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h" 
#include "Engine/NetSerialization.h" 
#include "PSJ_Character.generated.h"

class UInteracterComponent;
class UCameraComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class APawn;
class APSJ_Spaceship;

class ATurretBase_GT;
class APSJ_ToolBase;
class ATaskPawnBase;


USTRUCT()
struct FRelativeSpaceData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize100 RelativeLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator RelativeRotation = FRotator::ZeroRotator;

	UPROPERTY()
	AActor* BaseActor = nullptr;

	UPROPERTY()
	bool bIsAnchored = false;
};

UCLASS()
class TEAMSPACEPROJECT_API APSJ_Character : public ACharacter
{
	GENERATED_BODY()

public:
	APSJ_Character();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TObjectPtr<UInteracterComponent> InteracterComponent = nullptr;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Client_RestoreInput();

	virtual void PossessedBy(AController* NewController) override;
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;

	void ForceClearAnchoring();
	void Client_ForceCleanupImmediate();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestBoarding(ATaskPawnBase* TaskPawn);

private:

	bool bJustDisembarked = false;
	float DisembarkGraceTimer = 0.0f;

public:
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void TeleportToSpaceship(const FVector& DestLocation, const FRotator& DestRotation);

	UFUNCTION(Client, Reliable)
	void Client_TeleportAndReset(const FVector& DestLocation, const FRotator& DestRotation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	APawn* CurrentSpaceship = nullptr;
	void SetCurrentSpaceship(APawn* NewSpaceship);

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_RestoreInputRPC();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void Server_RequestPawnPossess(APawn* TargetPawn);

	void StartDisembarkState();

	void Client_LateInputRestore();

	UPROPERTY(EditDefaultsOnly, Category = "Tool")
	TSubclassOf<APSJ_ToolBase> ToolClassToSpawn; 

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Tool")
	APSJ_ToolBase* EquippedTool;


	void SetBaseActorData(AActor* NewBase);


	void ForceInputRecovery();


	void Input_ForceEject(const FInputActionValue& Value);


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TryForceEject(ATaskChair* TargetChair);


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RepairAction;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction | Repair", Replicated)
	bool bIsActivelyRepairing = false;


	UPROPERTY(EditAnywhere, Category = "Interaction | Repair")
	float RepairTraceLength = 2000.0f;


	UPROPERTY(EditAnywhere, Category = "Interaction | Repair")
	float RepairMaxDistance = 300.0f;


	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetInputVector(FVector2D NewInput);


	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_SetSprinting(bool bNewSprinting);

	bool TryUnboard();

protected:

	bool bIsRepairingInputDown = false;


	UPROPERTY()
	class ATaskChair* ClientRepairTarget = nullptr;


	UPROPERTY()
	class ATaskChair* ServerRepairTarget = nullptr;


	void Input_StartRepair(const FInputActionValue& Value);
	void Input_StopRepair(const FInputActionValue& Value);


	void UpdateRepairLogic();


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartRepair(ATaskChair* TargetChair);


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StopRepair();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* InteractAction;
	void InteractEnter(const FInputActionValue& Value);



	virtual void OnRep_Controller() override;


	UPROPERTY(EditAnywhere, Category = "Movement | Wall Detection")
	float WallTraceRadius = 40.0f; 

	UPROPERTY(EditAnywhere, Category = "Movement | Wall Detection")
	float WallTraceZOffset = 0.0f; 
	UPROPERTY(EditAnywhere, Category = "Movement | Wall Detection")
	bool bShowWallDebug = true; 

protected:


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetAnchoring(AActor* NewBase);

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float CheckDistance = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float MagBootsTraceRadius = 15.0f;


	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float MagBootsTraceHalfHeight = 30.0f;


	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float FloorHeightOffset = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Mag Boots")
	float AlignSpeed = 15.0f;

	UPROPERTY(EditAnywhere, Category = "Movement Stats")
	float FlyModeMaxSpeed = 600.0f;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Input", Replicated)
	FVector2D CurrentInputVector = FVector2D::ZeroVector;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement Input", Replicated)
	bool bIsSprinting = false;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SprintAction;


	void Input_SprintStart(const FInputActionValue& Value);
	void Input_SprintStop(const FInputActionValue& Value);

	UPROPERTY(Replicated)
	FRelativeSpaceData ReplicatedRelativeData;

	void UpdateMagBoots(float DeltaTime);

	UFUNCTION(Server, Unreliable, WithValidation)
	void Server_UpdateRelativeTransform(FVector NewRelLoc, FRotator NewRelRot);

	FVector CurrentFloorNormal = FVector::UpVector;
	UPROPERTY(Transient)
	AActor* LastFloorActor = nullptr;
	float DefaultMeshZ = 0.0f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* WheelAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	UCameraComponent* FPSCamera;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ForceEjectAction;


	UPROPERTY(EditAnywhere, Category = "Interaction | Force Eject")
	float ForceEjectSphereRadius = 40.0f;

	UPROPERTY(EditAnywhere, Category = "Interaction | Force Eject")
	float ForceEjectRange = 100.0f;


	UPROPERTY(EditAnywhere, Category = "Interaction | Force Eject")
	FVector ForceEjectSphereOffset = FVector(50.0f, 0.0f, 20.0f);

	float CurrentVerticalSpeed = 0.0f;

	void Move(const FInputActionValue& Value);
	void StopMove(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Wheel(const FInputActionValue& Value);
};