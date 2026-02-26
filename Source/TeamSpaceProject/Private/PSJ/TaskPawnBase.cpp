#include "PSJ/TaskPawnBase.h"
#include "PSJ_Character.h"
#include "EnhancedInputComponent.h" 
#include "InputTriggers.h"
#include "JHS/Interact/InteractableActorBase.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PSJ/TaskChair.h"

ATaskPawnBase::ATaskPawnBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ATaskPawnBase::BeginPlay()
{
	Super::BeginPlay();

	// 우주선에서 했던 것처럼, 블루프린트에 추가된 Arrow를 자동으로 찾습니다.
	TArray<UArrowComponent*> Arrows;
	GetComponents(Arrows);
	for (UArrowComponent* Arrow : Arrows)
	{
		if (Arrow->GetName().Contains(TEXT("Ride"))) RidePoint = Arrow;
		if (Arrow->GetName().Contains(TEXT("Arrow"))) ExitPoint = Arrow;
	}

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
	if (CurrentPilot && RidePoint)
	{
		CurrentPilot->SetActorEnableCollision(false);
		CurrentPilot->AttachToComponent(RidePoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		if (auto* CMC = CurrentPilot->GetCharacterMovement())
		{
			CMC->StopMovementImmediately();
			CMC->DisableMovement();
		}
	}
}

void ATaskPawnBase::Client_BoardingSuccess_Implementation()
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
	if (!CurrentPilot) return; // 여기 파일럿이 없으면 return 예외처리관련

    CurrentPilot->TryUnboard(); // 하차 시 입력이 즉시 복구되도록 함 (캐릭터가 좌표 이동 중에도 입력이 막히지 않도록)

	APSJ_Character* ExitingChar = CurrentPilot;
	AController* ShipController = GetController();

	CurrentPilot = nullptr;


	//if (LinkedSeat)
	//{
	//	APlayerController* _callerPC = ExitingChar ? Cast<APlayerController>(ExitingChar->GetController()) : nullptr;
	//	APlayerState* _callerPS = _callerPC ? _callerPC->GetPlayerState<APlayerState>() : nullptr;
	//	int32 _callerPlayerId = _callerPS ? _callerPS->GetPlayerId() : -1;

	//	if (ATaskChair* Chair = Cast<ATaskChair>(LinkedSeat))
	//	{
	//		Chair->OnInteractExit(_callerPlayerId, nullptr);
	//	}
	//	LinkedSeat = nullptr; 
	//}


	FVector SpawnLoc = ExitPoint ? ExitPoint->GetComponentLocation() : (RidePoint ? RidePoint->GetComponentLocation() : GetActorLocation());
	FRotator SpawnRot = ExitPoint ? ExitPoint->GetComponentRotation() : (RidePoint ? RidePoint->GetComponentRotation() : GetActorRotation());

	// 1. 하차 위치 설정 및 우주선에 다시 부착
	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);
	ExitingChar->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	ExitingChar->SetReplicateMovement(false);

	// 2. [중요] 캐릭터 데이터 동기화 및 '하차 유예 상태' 시작
	ExitingChar->SetBaseActorData(this);
	ExitingChar->StartDisembarkState(); // 클라이언트가 좌표를 덮어쓰지 못하게 보호함

	ExitingChar->SetActorEnableCollision(true);
	ExitingChar->SetActorHiddenInGame(false);

	Client_DisembarkSuccess(ExitingChar, SpawnLoc, SpawnRot);

	if (ShipController) ShipController->Possess(ExitingChar);
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