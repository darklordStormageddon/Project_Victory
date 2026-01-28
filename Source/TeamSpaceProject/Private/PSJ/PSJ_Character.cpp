#include "PSJ_Character.h"
#include "PSJ_Spaceship.h"
#include "PSJ_ShipCockpit.h"
#include "YSH/TurretBase_GT.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"
#include "JHS/GameControl/StateData/GameStateStructs.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraTypes.h" 

APSJ_Character::APSJ_Character()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();



	AJHSGameState* _outGameState = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameState(_outGameState))
		return;
	
	UTurretStateGroup* _turretStateGroup = _outGameState->GetTurretStateGroup();
	_turretStateGroup->SetInfiniteMagMode(true);

	TArray<TObjectPtr<AActor>> _chairArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APSJ_ShipCockpit::StaticClass(), _chairArray);
	for (TObjectPtr<AActor> _chairActor : _chairArray)
	{
		TObjectPtr<APSJ_ShipCockpit> _chair = Cast<APSJ_ShipCockpit>(_chairActor);
		if (!_chair)
		{
			//UE_LOG(LogTemp, Error, TEXT("TurretStateGroup: TurretStand not found. %s"), *_chairActor->GetName());
			continue;
		}

		if (_chair->TargetSpaceship == nullptr)
		{
			_turretStateGroup->SetTurretChair(_chair);
			break;
		}
	}

	_turretStateGroup->TryEquipTurret(E_TURRET_POSITION::Main, E_AMMO_TYPE::Bullet, nullptr);



	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = FlyModeMaxSpeed;

	FPSCamera = FindComponentByClass<UCameraComponent>();
	if (GetMesh())
	{
		DefaultMeshZ = GetMesh()->GetRelativeLocation().Z;
	}

	// [추가] 앵커링 데이터 즉시 적용
	if (ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(false);
	}
}

void APSJ_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSJ_Character, ReplicatedRelativeData);
}

void APSJ_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 이미 우주선에 붙어있는 상태라면 리셋 금지
	if (ReplicatedRelativeData.BaseActor && ReplicatedRelativeData.bIsAnchored)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		SetReplicateMovement(false);

		CurrentInputVector = FVector2D::ZeroVector;
	}
	else
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		SetReplicateMovement(true);

		ReplicatedRelativeData.BaseActor = nullptr;
		ReplicatedRelativeData.bIsAnchored = false;
		CurrentInputVector = FVector2D::ZeroVector;
	}
}

void APSJ_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		FString DebugMsg = FString::Printf(TEXT("Controller: %s | InputMode: %s"),
			Controller ? *Controller->GetName() : TEXT("NULL"),
			(DefaultMappingContext) ? TEXT("Context Valid") : TEXT("Context Null"));

		GEngine->AddOnScreenDebugMessage(10, 0.0f, FColor::Cyan, DebugMsg);

		if (CurrentInputVector.IsNearlyZero() == false)
		{
			GEngine->AddOnScreenDebugMessage(11, 0.0f, FColor::Green, TEXT("KEYBOARD INPUT DETECTED"));
		}
	}

	// 조종 중이면 로직 패스
	if (Controller && IsLocallyControlled() && CurrentSpaceship)
	{
		// ...
	}

	// 1. 앵커링 관리
	AActor* ParentActor = GetAttachParentActor();
	bool bShouldBeAttached = (ReplicatedRelativeData.BaseActor != nullptr);

	if (bShouldBeAttached && ParentActor != ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		SetReplicateMovement(false);
	}
	else if (!bShouldBeAttached && ParentActor)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		SetReplicateMovement(true);
	}

	// 2. 이동 로직
	if (GetAttachParentActor())
	{
		// [수정] 클라이언트뿐만 아니라 서버도 이동 로직에 관여하도록 변경 가능하나, 
		// 일단 클라이언트 예측 이동을 우선시합니다.
		if (IsLocallyControlled())
		{
			if (!CurrentInputVector.IsNearlyZero())
			{
				FVector LocalDir = FVector(CurrentInputVector.Y, CurrentInputVector.X, 0.0f);
				FVector DesiredMove = LocalDir * FlyModeMaxSpeed * DeltaTime;

				// [★핵심 해결책★] bSweep(충돌검사)를 false로 변경
				// 바닥이나 우주선 벽에 닿아있을 때 물리 엔진이 이동을 막는 현상(Stuck)을 방지합니다.
				AddActorLocalOffset(DesiredMove, false);

				// Sweep을 껐으므로 복잡한 StepUp/Slide 로직은 주석 처리하거나 건너뜁니다.
				// 자석 부츠(UpdateMagBoots)가 높이를 맞춰주므로 바닥을 뚫지 않습니다.
				/*
				if (Hit.IsValidBlockingHit())
				{
					// ... (기존 충돌 처리 로직 생략) ...
				}
				*/
			}

			if (DeltaTime > 0.0f)
			{
				FVector TargetVel = CurrentInputVector.IsNearlyZero() ? FVector::ZeroVector : (GetActorForwardVector() * CurrentInputVector.Y + GetActorRightVector() * CurrentInputVector.X) * FlyModeMaxSpeed;
				GetCharacterMovement()->Velocity = TargetVel;
			}

			UpdateMagBoots(DeltaTime);

			if (!HasAuthority())
			{
				Server_UpdateRelativeTransform(GetRootComponent()->GetRelativeLocation(), GetRootComponent()->GetRelativeRotation());
			}
			else
			{
				ReplicatedRelativeData.RelativeLocation = GetRootComponent()->GetRelativeLocation();
				ReplicatedRelativeData.RelativeRotation = GetRootComponent()->GetRelativeRotation();
			}
		}
		else
		{
			// 시뮬레이티드 프록시(다른 클라)는 서버에서 받은 상대 좌표를 적용
			SetActorRelativeLocation(ReplicatedRelativeData.RelativeLocation);
			SetActorRelativeRotation(ReplicatedRelativeData.RelativeRotation);
		}
	}
	else
	{
		UpdateMagBoots(DeltaTime);

		if (IsLocallyControlled() && !CurrentInputVector.IsNearlyZero())
		{
			FVector WorldDir = GetActorForwardVector() * CurrentInputVector.Y + GetActorRightVector() * CurrentInputVector.X;
			AddMovementInput(WorldDir);
		}
	}
}

// [신규] 변수 세팅 함수
void APSJ_Character::SetBaseActorData(AActor* NewBase)
{
	ReplicatedRelativeData.BaseActor = NewBase;
	ReplicatedRelativeData.bIsAnchored = (NewBase != nullptr);

	// 입력 벡터는 초기화하되, 이동 기능 자체는 끄지 않음
	CurrentInputVector = FVector2D::ZeroVector;
}

// [신규] 입력 강제 복구 함수 (핵심 해결책)
void APSJ_Character::ForceInputRecovery()
{
	// 내 컴퓨터의 0번 컨트롤러(플레이어)를 찾음
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		// 1. 입력 시스템(Enhanced Input) 가져오기
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 기존 매핑(우주선 키 등) 제거하고 내 키(WASD) 추가
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		// 2. [중요] 엔진에게 "이 컨트롤러의 입력을 이 액터가 받겠다"고 선언
		// 내부적으로 InputComponent를 생성하고 컨트롤러 스택에 푸시합니다.
		EnableInput(PC);

		// 3. [중요] 키 바인딩(Jump, Move 등) 연결
		if (InputComponent)
		{
			SetupPlayerInputComponent(InputComponent);
		}

		// 4. 입력 모드 강제 설정 (UI 닫기 포함)
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		//UE_LOG(LogTemp, Warning, TEXT("[Debug] ForceInputRecovery: Input Forced & Context Added!"));
	}
}

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	// 1. 타인(Simulated Proxy)이면서 이미 서버에 의해 붙어있는 상태라면 연산 최적화를 위해 패스
	if (!IsLocallyControlled() && GetAttachParentActor())
	{
		return;
	}

	// 2. 바닥 감지를 위한 레이캐스트(Sweep) 준비
	FHitResult FinalHit;
	bool bFoundValidHit = false;
	FVector Start = GetActorLocation();

	// 발바닥 방향 결정 (붙어있으면 부모 기준, 아니면 내 기준, 우주선 근처면 우주선 기준)
	FVector TraceDir = -GetActorUpVector();
	if (GetAttachParentActor())
	{
		TraceDir = -GetAttachParentActor()->GetActorUpVector();
	}
	else if (CurrentSpaceship)
	{
		TraceDir = -CurrentSpaceship->GetActorUpVector();
	}

	FVector End = Start + (TraceDir * CheckDistance);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(MagBootsTraceRadius * 0.8f);

	// 3. 바닥 감지 실행
	bool bHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_GameTraceChannel5, SphereShape, Params);

	if (bHit)
	{
		// 우선순위 결정: 기존에 밟고 있던 바닥이 감지되면 그걸 유지
		for (const FHitResult& Result : HitResults)
		{
			if (LastFloorActor && Result.GetActor() == LastFloorActor)
			{
				FinalHit = Result;
				bFoundValidHit = true;
				break;
			}
		}
		// 없으면 첫 번째 감지된 바닥 선택
		if (!bFoundValidHit && HitResults.Num() > 0)
		{
			FinalHit = HitResults[0];
			bFoundValidHit = true;
		}
	}

	// 4. [핵심 수정] 데이터 갱신 (서버 OR 클라이언트 본인)
	// 클라이언트도 스스로 판단하여 BaseActor를 세팅하게 함으로써 텔레포트 현상 방지
	bool bCanUpdateData = HasAuthority() || IsLocallyControlled();

	AActor* NewFloorActor = bFoundValidHit ? FinalHit.GetActor() : nullptr;

	if (bCanUpdateData)
	{
		if (NewFloorActor)
		{
			// 바닥 갱신 (즉시 Attach 유도)
			if (ReplicatedRelativeData.BaseActor != NewFloorActor)
			{
				ReplicatedRelativeData.BaseActor = NewFloorActor;
				ReplicatedRelativeData.bIsAnchored = true;
			}
		}
		else
		{
			// 바닥 놓침
			if (IsLocallyControlled() || HasAuthority())
			{
				ReplicatedRelativeData.BaseActor = nullptr;
				ReplicatedRelativeData.bIsAnchored = false;
			}
		}
	}

	// 5. 물리적 위치/회전 보정
	if (bFoundValidHit && NewFloorActor)
	{
		LastFloorActor = NewFloorActor;
		CurrentFloorNormal = NewFloorActor->GetActorUpVector();

		if (GetAttachParentActor())
		{
			// 높이 보정
			float TargetHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			float ActualDistance = FVector::DotProduct(FinalHit.ImpactPoint - GetActorLocation(), TraceDir);

			if (ActualDistance <= 0.0f) ActualDistance = FinalHit.Distance + MagBootsTraceRadius;

			float HeightError = ActualDistance - TargetHeight;

			if (FMath::Abs(HeightError) > 1.0f)
			{
				float InterpSpeed = (HeightError > 0) ? 20.0f : 50.0f;
				float MoveZ = FMath::FInterpTo(0.0f, -HeightError, DeltaTime, InterpSpeed);
				AddActorWorldOffset(TraceDir * -MoveZ, false);
			}

			// 회전 보정
			FRotator CurrentRelRot = GetRootComponent()->GetRelativeRotation();
			FRotator TargetRelRot = FRotator(0.0f, CurrentRelRot.Yaw, 0.0f);
			SetActorRelativeRotation(FMath::RInterpTo(CurrentRelRot, TargetRelRot, DeltaTime, AlignSpeed));
		}
	}
}

void APSJ_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//UE_LOG(LogTemp, Warning, TEXT("[Debug] SetupPlayerInputComponent Called! Controller: %s"), *GetNameSafe(Controller));

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
				//UE_LOG(LogTemp, Warning, TEXT("[Debug] Mapping Context Added!"));
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APSJ_Character::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APSJ_Character::StopMove);
		}
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APSJ_Character::Look);
		if (InteractAction) EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APSJ_Character::Interact);
	}
}

void APSJ_Character::SetCurrentSpaceship(APawn* NewSpaceship)
{
	CurrentSpaceship = NewSpaceship;
}

void APSJ_Character::Interact(const FInputActionValue& Value)
{
	if (CurrentSpaceship && Controller)
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (APSJ_Spaceship* TargetShip = Cast<APSJ_Spaceship>(CurrentSpaceship))
			{
				TargetShip->SetPilot(this);

				if (HasAuthority())
				{
					ReplicatedRelativeData.bIsAnchored = false;
					ReplicatedRelativeData.BaseActor = nullptr;
				}

				GetCharacterMovement()->DisableMovement();
				DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
				PC->Possess(TargetShip);
			}
		}
	}
}

void APSJ_Character::Move(const FInputActionValue& Value)
{
	CurrentInputVector = Value.Get<FVector2D>();

	if (!CurrentInputVector.IsNearlyZero())
	{
		FString ModeString = UEnum::GetValueAsString(GetCharacterMovement()->MovementMode);
		FVector Vel = GetVelocity();

		//UE_LOG(LogTemp, Warning, TEXT("[Debug] Move Input Received: %s | Mode: %s | Velocity: %s | IsAnchored: %d"),
		//	*CurrentInputVector.ToString(),
		//	*ModeString,
		//	*Vel.ToString(),
		//	ReplicatedRelativeData.bIsAnchored);
	}
}

void APSJ_Character::StopMove(const FInputActionValue& Value)
{
	CurrentInputVector = FVector2D::ZeroVector;
}

void APSJ_Character::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (LookAxisVector.X != 0.0f)
	{
		AddActorLocalRotation(FRotator(0.0f, LookAxisVector.X, 0.0f));
	}

	if (FPSCamera && LookAxisVector.Y != 0.0f)
	{
		FRotator CurrentCamRot = FPSCamera->GetRelativeRotation();
		float NewPitch = CurrentCamRot.Pitch + (LookAxisVector.Y * -1.0f);
		NewPitch = FMath::Clamp(NewPitch, -80.0f, 80.0f);
		FPSCamera->SetRelativeRotation(FRotator(NewPitch, 0.0f, 0.0f));
	}
}

bool APSJ_Character::Server_UpdateRelativeTransform_Validate(FVector NewRelLoc, FRotator NewRelRot)
{
	return true;
}

void APSJ_Character::Server_UpdateRelativeTransform_Implementation(FVector NewRelLoc, FRotator NewRelRot)
{
	ReplicatedRelativeData.RelativeLocation = NewRelLoc;
	ReplicatedRelativeData.RelativeRotation = NewRelRot;

	if (ReplicatedRelativeData.BaseActor && GetAttachParentActor() == ReplicatedRelativeData.BaseActor)
	{
		SetActorRelativeLocation(NewRelLoc);
		SetActorRelativeRotation(NewRelRot);
	}
}

void APSJ_Character::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	Super::CalcCamera(DeltaTime, OutResult);
	if (FPSCamera)
	{
		OutResult.Location = FPSCamera->GetComponentLocation();
		OutResult.Rotation = FPSCamera->GetComponentRotation();
	}
}

bool APSJ_Character::Server_RequestBoarding_Validate(APSJ_Spaceship* ShipToBoard)
{
	return true;
}

void APSJ_Character::Server_RequestBoarding_Implementation(APSJ_Spaceship* ShipToBoard)
{
	if (!ShipToBoard) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		ShipToBoard->SetPilot(this);
		PC->Possess(ShipToBoard);
		ShipToBoard->Client_BoardingSuccess();
	}
}

void APSJ_Character::ForceClearAnchoring()
{
	ReplicatedRelativeData.BaseActor = nullptr;
	ReplicatedRelativeData.bIsAnchored = false;
	CurrentInputVector = FVector2D::ZeroVector;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	SetReplicateMovement(true);
}

void APSJ_Character::Client_ForceCleanupImmediate()
{
	ReplicatedRelativeData.BaseActor = nullptr;
	ReplicatedRelativeData.bIsAnchored = false;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}

	SetReplicateMovement(true);
	CurrentInputVector = FVector2D::ZeroVector;
}

void APSJ_Character::Client_RestoreInput()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void APSJ_Character::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (IsLocallyControlled() && Controller)
	{
		// 엔진 표준 함수로 입력 시스템 재시동
		PawnClientRestart();

		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;

			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				if (DefaultMappingContext)
				{
					Subsystem->AddMappingContext(DefaultMappingContext, 0);
				}
			}
		}

		// 혹시 모를 안전장치: 강제 인풋 복구 호출
		ForceInputRecovery();

		//UE_LOG(LogTemp, Warning, TEXT("[Debug] OnRep_Controller: PawnClientRestart Called. Input Restored."));
	}
}

void APSJ_Character::Client_LateInputRestore()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		//UE_LOG(LogTemp, Warning, TEXT("[Debug] LateInputRestore: Forcing Input Setup..."));
		PawnClientRestart();
		ForceInputRecovery();
	}
}

bool APSJ_Character::Server_RequestTurretBoarding_Validate(ATurretBase_GT* TurretToBoard, APSJ_ShipCockpit* LinkedCockpit)
{
	return true;
}

void APSJ_Character::Server_RequestTurretBoarding_Implementation(ATurretBase_GT* TurretToBoard, APSJ_ShipCockpit* LinkedCockpit)
{
	if (!TurretToBoard) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 1. 터렛에 조종사 정보 등록
		TurretToBoard->SetPilot(this, LinkedCockpit);

		// 2. 컨트롤러 빙의 (Possess) - 이제 캐릭터가 아닌 터렛을 조종
		PC->Possess(TurretToBoard);

		// 3. 클라이언트 화면/입력 전환 지시
		TurretToBoard->Client_BoardingSuccess();
	}
}