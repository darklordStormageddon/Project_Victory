#include "YSH/YSHPlayerBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "JHS/UI/UIInteracterable.h"


AYSHPlayerBase::AYSHPlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AYSHPlayerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AYSHPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYSHPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AYSHPlayerBase::EnterInteractable(UUIInteracterable* Interactable, AActor* NewViewTarget)
{
	UE_LOG(LogTemp, Warning, TEXT("EnterInteractable called"));

	if (!Interactable)
		return;

	if (bIsInteracting)
		return;

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	CurrentInteractable = Interactable;
	bIsInteracting = true;

	// 1) 원래 뷰타겟 저장
	SavedViewTarget = PC->GetViewTarget();
	bHasSavedViewTarget = SavedViewTarget.IsValid();

	// 2) 이동/시점 입력 잠금(일단 둘 다 잠금: 포탑 조종 입력은 추후 포탑 전용으로 라우팅)
	SetPlayerControlLock(true);

	// 3) 포탑 시점으로 전환
	if (NewViewTarget)
	{
		SwitchToViewTarget(NewViewTarget, 0.2f);
	}
}

void AYSHPlayerBase::ExitInteractable()
{
	if (!bIsInteracting)
		return;

	bIsInteracting = false;
	CurrentInteractable = nullptr;

	// 1) 카메라 원복
	RestoreViewTarget(0.2f);

	// 2) 이동/시점 입력 잠금 해제
	SetPlayerControlLock(false);

	bHasSavedViewTarget = false;
	SavedViewTarget.Reset();
}

void AYSHPlayerBase::SwitchToViewTarget(AActor* NewViewTarget, float BlendTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC || !NewViewTarget)
		return;

	FViewTargetTransitionParams Params;
	Params.BlendTime = BlendTime;

	PC->SetViewTarget(NewViewTarget, Params);
}

void AYSHPlayerBase::RestoreViewTarget(float BlendTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
		return;

	AActor* Target = bHasSavedViewTarget ? SavedViewTarget.Get() : nullptr;
	if (!Target)
	{
		// fallback: 자기 자신(또는 Pawn)으로
		Target = this;
	}

	FViewTargetTransitionParams Params;
	Params.BlendTime = BlendTime;

	PC->SetViewTarget(Target, Params);
}

void AYSHPlayerBase::SetPlayerControlLock(bool bLock)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->SetIgnoreMoveInput(bLock);
		PC->SetIgnoreLookInput(bLock);
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		if (bLock)
		{
			MoveComp->StopMovementImmediately();
			MoveComp->DisableMovement();
		}
		else
		{
			MoveComp->SetMovementMode(MOVE_Walking);
		}
	}
}