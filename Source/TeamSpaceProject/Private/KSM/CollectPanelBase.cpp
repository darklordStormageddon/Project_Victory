// Fill out your copyright notice in the Description page of Project Settings.
#include "KSM/CollectPanelBase.h"
#include "PSJ/PSJ_Character.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "JHS/Event/EventManager.h"
#include "JHS/Event/CommonEventBase.h"
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

void ACollectPanelBase::UpdateToolUsage_Implementation()
{
	if (_collectStateGroup == nullptr)
		return;
	//UE_LOG(LogTemp, Log, TEXT("ACollectPanelBase::UpdateToolUsage_Implementation - Tool Type: %s"), *CommonEnums::GetEnum2FString<E_COLLECT_TOOL_TYPE>(_Tool_Type));
	if (!_collectStateGroup->TrySelectTool(_Tool_Type))
		return;
}

void ACollectPanelBase::UpdateDurability_Implementation()
{
	if (_collectStateGroup == nullptr)
		return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	if (!_collectStateGroup->TryUseTool(UStaticFunctionLibrary::GetAssignedPlayerId(), _Tool_Type, DeltaTime, _OutToolDamage))
		return;
}

void ACollectPanelBase::Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot)
{
	Super::Client_BoardingSuccess_Implementation(BoardingPilot);

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

void ACollectPanelBase::Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector ExitLoc, FRotator ExitRot)
{
	if (!ExitingPilot) return;

	if (UCharacterMovementComponent* CMC = ExitingPilot->GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->SetMovementMode(MOVE_Custom);
	}

	ExitingPilot->MoveIgnoreActorRemove(this);
	this->MoveIgnoreActorRemove(ExitingPilot);

	FVector SafeExitLoc = ExitLoc + GetActorUpVector() * 15.0f;
	ExitingPilot->SetActorLocationAndRotation(SafeExitLoc, ExitRot, false, nullptr, ETeleportType::TeleportPhysics);

	ExitingPilot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	ExitingPilot->StartDisembarkState();
	ExitingPilot->SetBaseActorData(this);

}

