// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "JHS/UI/UIBase.h"
#include "JHS/GameControl/CommonEnums.h"

#include "InteractableComponent.generated.h"

class APlayerController;
class AJHSPlayerController;
class UInteracterComponent;
class UUIManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractEnterAction, int32, CallerPlayerId, UUIBase*, OpenedUI);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractExitAction, int32, CallerPlayerId, UUIBase*, ClosedUI);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractInterruptAction);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UInteractableComponent();

private:
	UPROPERTY()
	TObjectPtr<UUIManager> _uiManager = nullptr;

	UPROPERTY()
	TObjectPtr<USphereComponent> _collisionComponent = nullptr;

	UPROPERTY()
	TObjectPtr<UInteracterComponent> _interacter = nullptr;

	/** 로컬 PlayerController 캐시 (GetPlayerController 호출 최소화). */
	TWeakObjectPtr<AJHSPlayerController> _cachedLocalPlayerController;

	bool _isInteract = false;

	UPROPERTY()
	TObjectPtr<UInteracterComponent> _InterruptInteracter = nullptr;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact|Debug")
	bool _isDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float _collisionRadius = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	E_INTERACT_TYPE _interactType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool _isInterrupt = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	E_UI_TYPE _interactUIType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	bool _isWorldSpaceUI = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FVector _worldUIRelativeLocation = FVector(0, 0, 150);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float _worldUIScale = 1.0f;

public:
	UPROPERTY(BlueprintAssignable, Category = "Interacter|Events")
	FOnInteractEnterAction OnInteractEnterAction;

	UPROPERTY(BlueprintAssignable, Category = "Interacter|Events")
	FOnInteractExitAction OnInteractExitAction;

	UPROPERTY(BlueprintAssignable, Category = "Interacter|Events")
	FOnInteractInterruptAction OnInteractInterruptAction;

public:
	E_INTERACT_TYPE GetInteractType() { return _interactType; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	/** Client RPC 수신 측에서 호출. 로컬 Pawn의 Interacter에 OnInteractable 호출 (복제 타이밍 무관). */
	void ExecuteTriggerEnterForLocalPlayer(E_INTERACT_TYPE InteractType);

	/** Client RPC 수신 측에서 호출. 로컬 Pawn의 Interacter에 OnDisInteractable 호출 (복제 타이밍 무관). */
	void ExecuteTriggerExitForLocalPlayer();

	// 서버 전용: 트리거 진입/이탈 로직 (InteracterComponent의 Server RPC에서 호출, 레벨 액터는 owning connection 없어 직접 Server RPC 불가)
	void ExecuteServerTriggerEnter(AActor* OtherActor);
	void ExecuteServerTriggerExit(AActor* OtherActor);

private:
	/** 호출자 구별: 서버에서 발급·복제한 AssignedPlayerId (FPlayerStateData와 동일). */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnTriggerEnter(int32 CallerAssignedPlayerId, E_INTERACT_TYPE InteractType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnTriggerExit(int32 CallerAssignedPlayerId);

	/** 캐시된 로컬 PlayerController 반환 또는 조회 후 캐시. */
	bool GetOrCacheLocalPlayerController(AJHSPlayerController*& OutController);

	/** 멀티캐스트 수신 측: AssignedPlayerId에 해당하는 PC를 찾고, 그 PC가 로컬일 때만 반환. (로컬 PC 우선 조회에 의존하지 않음) */
	bool FindLocalPlayerControllerByAssignedId(UWorld* World, int32 AssignedPlayerId, AJHSPlayerController*& OutController);

public:
	void InitializeUIInteractable(bool IsDebugDraw, float InteractRadius, E_INTERACT_TYPE InteractType, E_UI_TYPE InteractUIType, bool IsWorldSpaceUI, FVector WorldUIRelativeLocation, float WorldUIScale);

	bool TryInteract(APlayerController* CallerController, bool& OutIsInterupt, bool& IsCloseUI);

private:
	UFUNCTION(Server, Reliable)
	void ServerOpenWorldUI(int32 CallerPlayerId, E_UI_TYPE UIType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOpenWorldUI(int32 CallerPlayerId, E_UI_TYPE UIType);

	UFUNCTION(Server, Reliable)
	void ServerCloseWorldUI(int32 CallerPlayerId, E_UI_TYPE UIType);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastCloseWorldUI(int32 CallerPlayerId, E_UI_TYPE UIType);

	void ChangeInteractState(bool IsInteract, int32 CallerPlayerId);

	bool TryGetUIManager(UUIManager*& OutUIManager);
};
