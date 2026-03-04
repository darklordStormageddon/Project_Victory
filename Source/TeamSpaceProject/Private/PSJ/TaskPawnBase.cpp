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
		// 1. 의자 찾기 로직 (기존과 동일)
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

		// 2. [수정] 모든 물리/상태 변화를 다음 프레임으로 지연 실행
		// 이렇게 하면 InteracterComponent의 TryInteractInput 함수가 안전하게 종료된 후 콜리전이 꺼집니다.
		GetWorld()->GetTimerManager().SetTimerForNextTick([WeakThis, WeakPilot, WeakChair]()
			{
				if (WeakThis.IsValid() && WeakPilot.IsValid() && WeakChair.IsValid())
				{
					// [여기서 점유 상태 변경]
					// 서버 캐릭터의 경우, 함수 호출 스택이 완전히 빠져나간 뒤 실행되므로 안전합니다.
					if (WeakThis->HasAuthority())
					{
						WeakChair->bIsOccupied = true;
						WeakChair->OnRep_IsOccupied();
					}

					// 캐릭터 위치 이동 및 부착 (기존 로직)
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
			FoundChair->OnRep_IsOccupied(); // 콜리전 즉시 활성화
		}
		SpawnLoc = FoundChair->GetActorTransform().TransformPosition(FoundChair->SeatDisembarkOffset);
		SpawnRot = FoundChair->GetActorRotation();
	}

	if (UCharacterMovementComponent* CMC = ExitingChar->GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->Velocity = FVector::ZeroVector;
	}

	// [핵심 변경 1] 허공에 버리지(Detach) 않고, 하차할 기기에 명시적으로 묶어둡니다(Attach).
	ExitingChar->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 서버 데이터 상으로도 캐릭터가 기기에 Anchored 되어 있음을 기록합니다.
	ExitingChar->SetBaseActorData(this);
	ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	ExitingChar->SetReplicateMovement(true);

	ExitingChar->StartDisembarkState();
	ExitingChar->SetActorEnableCollision(true);
	ExitingChar->SetActorHiddenInGame(false);

	// [핵심 변경 2] 현재 기기 기준의 '로컬 좌표'를 구해서 클라이언트로 전송합니다.
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

	// [핵심 변경 3] 완전히 분리해 버리는 Client_ForceCleanupImmediate() 함수 호출을 삭제했습니다.
	// 네트워크 지연 시간 동안 기기가 이동했더라도 캐릭터가 붙어서 따라가게 됩니다.

	// 1. 서버에서 받은 로컬 좌표를, '클라이언트 패킷 수신 시점'의 최신 기기 월드 좌표로 변환
	FVector TargetWorldLoc = this->GetActorTransform().TransformPosition(LocalLoc);
	FRotator TargetWorldRot = this->GetActorTransform().TransformRotation(LocalRot.Quaternion()).Rotator();

	// 2. 텔레포트 전, 확실하게 기기에 Attach 시킵니다.
	ExitingPilot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	ExitingPilot->SetActorLocationAndRotation(TargetWorldLoc, TargetWorldRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 3. 콜리전 및 무브먼트 보간 데이터 초기화 (클라이언트 단 안전장치)
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

	// 4. 자석 부츠 1회 강제 실행
	// 이제 기기에 제대로 Attach 되어 있기 때문에, 기기가 기울어져 있어도 올바른 UpVector를 참조합니다.
	ExitingPilot->ForceExecuteMagBoots();

	// 5. 입력 즉시 복구
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