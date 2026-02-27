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
		// 1. 나를 가리키는 의자(TaskChair) 찾기 (이 작업은 즉시 수행)
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

		// 안전한 지연 실행을 위한 약참조(Weak Pointer) 설정
		TWeakObjectPtr<ATaskPawnBase> WeakThis(this);
		TWeakObjectPtr<APSJ_Character> WeakPilot(CurrentPilot);
		TWeakObjectPtr<ATaskChair> WeakChair(FoundChair);

		// 2. 물리 충돌 변경 및 이동 로직을 다음 프레임(Next Tick)으로 지연!
		GetWorld()->GetTimerManager().SetTimerForNextTick([WeakThis, WeakPilot, WeakChair]()
			{
				if (WeakThis.IsValid() && WeakPilot.IsValid())
				{
					// 캐릭터 위치 이동
					if (WeakChair.IsValid())
					{
						WeakPilot->SetActorLocationAndRotation(WeakChair->GetActorLocation(), WeakChair->GetActorRotation());
					}

					// 부착 및 충돌 해제
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
	// 부모 클래스에서는 기본적인 카메라 전환이나 공통 UI 처리 등을 할 수 있습니다.
	// (특정 조작키 IMC 할당은 자식 클래스에서 오버라이드하여 수행합니다.)
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
	if (!CurrentPilot) return; // 파일럿이 없으면 return 예외처리

	CurrentPilot->TryUnboard(); // 하차 시 입력이 즉시 복구되도록 함

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

	// 의자의 오프셋을 기준으로 하차 위치 계산
	if (FoundChair)
	{
		SpawnLoc = FoundChair->GetActorTransform().TransformPosition(FoundChair->SeatDisembarkOffset);
	}

	if (UCharacterMovementComponent* CMC = ExitingChar->GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->Velocity = FVector::ZeroVector;
	}

	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ExitingChar->ForceClearAnchoring();
	ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);

	ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	ExitingChar->SetReplicateMovement(true);

	//ExitingChar->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	//ExitingChar->SetBaseActorData(this);
	ExitingChar->StartDisembarkState();

	ExitingChar->SetActorEnableCollision(true);
	ExitingChar->SetActorHiddenInGame(false);

	Client_DisembarkSuccess(ExitingChar, SpawnLoc, SpawnRot);

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


void ATaskPawnBase::Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector ExitLoc, FRotator ExitRot)
{
	if (!ExitingPilot) return;

	ExitingPilot->MoveIgnoreActorRemove(this);
	this->MoveIgnoreActorRemove(ExitingPilot);

	ExitingPilot->Client_ForceCleanupImmediate();

	// 1. 위치 텔레포트
	ExitingPilot->SetActorLocationAndRotation(ExitLoc, ExitRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 2. 무브먼트 보간 데이터 초기화 (이전 팁 유지)
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

	// 3. [핵심] 타이머 딜레이 삭제하고 자석 부츠 1회 강제 실행!
	ExitingPilot->ForceExecuteMagBoots();

	// 4. 입력 즉시 복구
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