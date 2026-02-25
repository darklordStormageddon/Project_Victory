// Fill out your copyright notice in the Description page of Project Settings.
#include "KSM/CollectPanelBase.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "Net/UnrealNetwork.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ACollectPanelBase::ACollectPanelBase()
{
	bReplicates = true;
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACollectPanelBase::BeginPlay()
{
	Super::BeginPlay();
	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;

	_collectStateGroup = _outGameState->GetCollectStateGroup();
}

void ACollectPanelBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(ACollectPanelBase, _Tool_Type);
		DOREPLIFETIME(ACollectPanelBase, _OutToolDamage);
}

// Called every frame
void ACollectPanelBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACollectPanelBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACollectPanelBase::UpdateDurabilityAndDamage_Implementation()
{
	if (_collectStateGroup == nullptr)
		return;

	if (!_collectStateGroup->TrySelectTool(_Tool_Type))
		return;

	// 사용 예시
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (!_collectStateGroup->TryUseTool(_Tool_Type, DeltaTime, _OutToolDamage))
		return;
}

void ACollectPanelBase::Client_BoardingSuccess_Implementation()
{
	Super::Client_BoardingSuccess_Implementation();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (PanelMappingContext)
			{
				Subsystem->AddMappingContext(PanelMappingContext, 0);
			}
		}
	}
}

