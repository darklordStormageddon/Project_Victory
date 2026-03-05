#include "PSJ/TaskPawnBase.h"
#include "PSJ_Character.h"
#include "EnhancedInputComponent.h" 
#include "InputTriggers.h"
#include "JHS/Interact/InteractableActorBase.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PSJ/TaskChair.h"

ATaskPawnBase::ATaskPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ATaskPawnBase::BeginPlay()
{
	Super::BeginPlay();

	UEventManager* _eventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_eventManager) || _eventManager == nullptr)
		return;

	_eventHandleOnEndStage = _eventManager->AddListener<UEventOnEndStage>(
		[this](UEventOnEndStage* Event)
		{
			OnEndStage(Event);
		}
    );
}

void ATaskPawnBase::SetPilot(ACharacter* Character)
{
	CurrentPilot = Cast<APSJ_Character>(Character);

	if (CurrentPilot)
	{
		ATaskChair* FoundChair = Cast<ATaskChair>(LinkedSeat);
		if (!FoundChair)
		{
			TArray<AActor*> AllChairs;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATaskChair::StaticClass(), AllChairs);
			for (AActor* Actor : AllChairs)
			{
				ATaskChair* Chair = Cast<ATaskChair>(Actor);
				if (Chair && Chair->GetTargetTaskPawn() == this)
				{
					FoundChair = Chair;
					break;
				}
			}
		}

		TWeakObjectPtr<ATaskPawnBase> WeakThis(this);
		TWeakObjectPtr<APSJ_Character> WeakPilot(CurrentPilot);
		TWeakObjectPtr<ATaskChair> WeakChair(FoundChair);


		GetWorld()->GetTimerManager().SetTimerForNextTick([WeakThis, WeakPilot, WeakChair]()
			{
				if (WeakThis.IsValid() && WeakPilot.IsValid() && WeakChair.IsValid())
				{

					if (WeakThis->HasAuthority())
					{
						WeakChair->bIsOccupied = true;
						WeakChair->OnRep_IsOccupied();
					}

					WeakPilot->SetActorLocationAndRotation(WeakChair->GetActorLocation(), WeakChair->GetActorRotation());
					WeakPilot->AttachToActor(WeakThis.Get(), FAttachmentTransformRules::KeepWorldTransform);
					WeakPilot->SetActorEnableCollision(false);

					if (auto* CMC = WeakPilot->GetCharacterMovement())
					{
						CMC->StopMovementImmediately();
						CMC->DisableMovement();
					}
				}
		});
	}
}

void ATaskPawnBase::Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot)
{

}

void ATaskPawnBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		
		if (IA_Interact) EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &ATaskPawnBase::Input_Exit);
	}
}

void ATaskPawnBase::DisembarkCharacter()
{
	if (!HasAuthority()) return;

	if (!CurrentPilot) return;

	CurrentPilot->TryUnboard();

	APSJ_Character* ExitingChar = CurrentPilot;
	AController* ShipController = GetController();

	CurrentPilot = nullptr;

	FVector SpawnLoc = GetActorLocation();
	FRotator SpawnRot = GetActorRotation();

	ATaskChair* FoundChair = Cast<ATaskChair>(LinkedSeat);
	if (!FoundChair)
	{
		TArray<AActor*> AllChairs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATaskChair::StaticClass(), AllChairs);

		for (AActor* Actor : AllChairs)
		{
			ATaskChair* Chair = Cast<ATaskChair>(Actor);
			if (Chair && Chair->GetTargetTaskPawn() == this)
			{
				FoundChair = Chair;
				break;
			}
		}
	}

	if (FoundChair)
	{
		if (HasAuthority())
		{
			FoundChair->bIsOccupied = false;
			FoundChair->OnRep_IsOccupied(); 
		}
		SpawnLoc = FoundChair->GetActorTransform().TransformPosition(FoundChair->SeatDisembarkOffset);
		SpawnRot = FoundChair->GetActorRotation();
	}

	if (UCharacterMovementComponent* CMC = ExitingChar->GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->Velocity = FVector::ZeroVector;
	}


	ExitingChar->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);

	ExitingChar->SetBaseActorData(this);
	ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	ExitingChar->SetReplicateMovement(true);

	ExitingChar->StartDisembarkState();
	ExitingChar->SetActorEnableCollision(true);
	ExitingChar->SetActorHiddenInGame(false);

	FVector LocalLoc = this->GetActorTransform().InverseTransformPosition(SpawnLoc);
	FRotator LocalRot = this->GetActorTransform().InverseTransformRotation(SpawnRot.Quaternion()).Rotator();

	Client_DisembarkSuccess(ExitingChar, LocalLoc, LocalRot);

	if (ShipController)
	{
		ShipController->Possess(ExitingChar);
	}
}

void ATaskPawnBase::Input_Exit(const FInputActionValue& Value)
{


	Server_RequestDisembark();
}

bool ATaskPawnBase::Server_RequestDisembark_Validate()
{
	return true;
}

void ATaskPawnBase::Server_RequestDisembark_Implementation()
{
	DisembarkCharacter();
}


void ATaskPawnBase::Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector LocalLoc, FRotator LocalRot)
{
	if (!ExitingPilot) return;

	ExitingPilot->MoveIgnoreActorRemove(this);
	this->MoveIgnoreActorRemove(ExitingPilot);


	FVector TargetWorldLoc = this->GetActorTransform().TransformPosition(LocalLoc);
	FRotator TargetWorldRot = this->GetActorTransform().TransformRotation(LocalRot.Quaternion()).Rotator();


	ExitingPilot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	ExitingPilot->SetActorLocationAndRotation(TargetWorldLoc, TargetWorldRot, false, nullptr, ETeleportType::TeleportPhysics);


	ExitingPilot->SetActorEnableCollision(true);
	if (UCharacterMovementComponent* CMC = ExitingPilot->GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->Velocity = FVector::ZeroVector;
		CMC->SetMovementMode(MOVE_Custom);
		CMC->bJustTeleported = true;
		if (CMC->HasPredictionData_Client())
		{
			CMC->ResetPredictionData_Client();
		}
	}

	ExitingPilot->StartDisembarkState();


	ExitingPilot->ForceExecuteMagBoots();

	ExitingPilot->ForceInputRecovery();
}

void ATaskPawnBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UEventManager* _eventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_eventManager) || _eventManager == nullptr)
		return;

	if (_eventHandleOnEndStage.IsValid())
	{
		_eventManager->DelListener<UEventOnEndStage>(_eventHandleOnEndStage);
		_eventHandleOnEndStage.Reset();
	}




	Super::EndPlay(EndPlayReason);

}

void ATaskPawnBase::OnEndStage(UEventOnEndStage* Event)
{
	if (Event == nullptr)
		return;

	DisembarkCharacter();
}