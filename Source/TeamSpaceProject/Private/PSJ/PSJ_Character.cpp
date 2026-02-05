#include "PSJ_Character.h"
#include "PSJ_Spaceship.h"
#include "PSJ_ShipCockpit.h"
#include "YSH/TurretBase_GT.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
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
	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();

	AJHSGameMode* _outGameMode = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
		return;

	_outGameMode->StartGame(this);

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

// [PSJ_Character.cpp]

void APSJ_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		UCharacterMovementComponent* CMC = GetCharacterMovement();
		FVector Vel = GetVelocity();
		FString ModeString = UEnum::GetValueAsString(CMC->MovementMode);

		// 1. 상세 속도 및 모드 출력
		FString DebugMsg = FString::Printf(TEXT("Vel: %s | Mode: %s | Speed: %.2f"),
			*Vel.ToString(), *ModeString, Vel.Size());
		GEngine->AddOnScreenDebugMessage(10, 0.0f, FColor::Yellow, DebugMsg);

		// 2. 엔진이 마음대로 모드를 바꿨는지 감시 및 강제 교정
		if (CMC->MovementMode == MOVE_Walking)
		{
			GEngine->AddOnScreenDebugMessage(12, 1.0f, FColor::Red, TEXT("CRITICAL: Mode flipped to Walking! Reverting..."));
			CMC->SetMovementMode(MOVE_Custom); // 강제로 다시 돌려놓음
		}
	}

	// 1. 공용 예외 처리
	if (!Controller || (IsLocallyControlled() && CurrentSpaceship)) return;

	// 2. 앵커링(부착) 상태 동기화
	AActor* ParentActor = GetAttachParentActor();
	if (ReplicatedRelativeData.BaseActor && ParentActor != ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		SetReplicateMovement(false);
	}
	else if (!ReplicatedRelativeData.BaseActor && ParentActor)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(true);
	}

	// 3. 로컬 컨트롤러 이동 로직
	if (IsLocallyControlled())
	{
		if (!CurrentInputVector.IsNearlyZero())
		{
			// 입력 벡터 -> 월드 이동 벡터 변환
			FVector LocalDir = FVector(CurrentInputVector.Y, CurrentInputVector.X, 0.0f);
			FVector WorldDir = GetActorQuat().RotateVector(LocalDir);

			// =================================================================================
			// [핵심 기능 복구] 경사면 이동 투영 (Slope Projection)
			// 바닥에 붙어있다면, 이동 방향을 바닥 경사면에 맞춰서 '비행기 이륙하듯' 꺾어줍니다.
			// 이렇게 하면 계단을 들이받지 않고 타고 올라갑니다.
			// =================================================================================
			if (ReplicatedRelativeData.bIsAnchored && !CurrentFloorNormal.IsZero())
			{
				FVector SlopeDir = FVector::VectorPlaneProject(WorldDir, CurrentFloorNormal);
				WorldDir = SlopeDir.GetSafeNormal();
			}

			FVector MoveDelta = WorldDir * FlyModeMaxSpeed * DeltaTime;

			FHitResult MoveHit;
			GetCharacterMovement()->SafeMoveUpdatedComponent(
				MoveDelta,
				GetActorRotation(),
				true,
				MoveHit
			);

			if (MoveHit.IsValidBlockingHit())
			{
				FVector SlideVector = FVector::VectorPlaneProject(MoveDelta, MoveHit.Normal);
				float RemainingPercent = 1.0f - MoveHit.Time;
				GetCharacterMovement()->SafeMoveUpdatedComponent(
					SlideVector * RemainingPercent,
					GetActorRotation(),
					true,
					MoveHit
				);
			}
		}

		// 4. 자석 부츠 (바닥 감지 및 높이 보정)
		UpdateMagBoots(DeltaTime);

		// 5. 서버 동기화
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
		// [Simulated Proxy]
		if (ReplicatedRelativeData.BaseActor)
		{
			SetActorRelativeLocation(ReplicatedRelativeData.RelativeLocation);
			SetActorRelativeRotation(ReplicatedRelativeData.RelativeRotation);
		}
	}
}

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	if (!IsLocallyControlled()) return;

	// 1. 방향 설정 (중력 방향)
	FVector GravityUpDir = FVector::UpVector;
	if (GetAttachParentActor()) GravityUpDir = GetAttachParentActor()->GetActorUpVector();
	FVector DownDir = -GravityUpDir;

	// -----------------------------------------------------------
	// [필수 수정] 캡슐 시작점 보정 (땅속 파묻힘 방지)
	// -----------------------------------------------------------
	float MyHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float TraceHalfHeight = MagBootsTraceHalfHeight; // 에디터 설정값 (20.0)

	// 트레이스 캡슐의 바닥을 내 발바닥 높이에 정확히 맞춤
	float HeightDiff = TraceHalfHeight - MyHalfHeight;
	FVector StartOffset = GravityUpDir * (HeightDiff + 0.1f); // 0.1f는 미세한 겹침 방지

	FVector Start = GetActorLocation() + StartOffset;

	// 예측 트레이스 (이동 중일 때 앞쪽 미리 감지)
	FVector Velocity = GetVelocity();
	if (Velocity.SizeSquared() > 10.0f)
	{
		FVector PredictionOffset = Velocity.GetSafeNormal() * MagBootsTraceRadius;
		PredictionOffset = FVector::VectorPlaneProject(PredictionOffset, GravityUpDir);
		Start += PredictionOffset;
	}
	FVector End = Start + (DownDir * CheckDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(MagBootsTraceRadius, MagBootsTraceHalfHeight);
	FQuat ShapeRotation = FRotationMatrix::MakeFromZ(GravityUpDir).ToQuat();

	// 2. 트레이스 및 판별 (채널 OR 태그)
	bool bFoundValidFloor = false;

	// (1) 평평한 바닥판 감지 (채널: SpaceshipFloor) - 태그 불필요
	bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, ShapeRotation, ECC_Spaceship_Floor, CapsuleShape, Params);

	if (bHit)
	{
		bFoundValidFloor = true; // 전용 채널이면 무조건 합격
	}
	else
	{
		// (2) 계단 감지 (채널: Visibility) - 태그 필수
		bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, ShapeRotation, ECC_Visibility, CapsuleShape, Params);

		if (bHit && Hit.GetActor())
		{
			if (Hit.GetActor()->ActorHasTag(TEXT("Stairs")))
			{
				bFoundValidFloor = true; // "Stairs" 태그가 있으면 합격
			}
			else
			{
				// 태그도 없고 전용 채널도 아님 (우주선 바닥 등) -> 불합격 (벽 취급)
				bFoundValidFloor = false;
			}
		}
	}

	// 3. 결과 처리
	if (bFoundValidFloor && Hit.GetActor())

	{

		// [수정] 새로운 바닥을 발견했을 때
		if (GetAttachParentActor() != Hit.GetActor())
		{
			// 1. 클라이언트 우선 적용 (즉각적인 반응성을 위해)
			AttachToActor(Hit.GetActor(), FAttachmentTransformRules::KeepWorldTransform);
			GetCharacterMovement()->SetMovementMode(MOVE_Custom);
			ReplicatedRelativeData.BaseActor = Hit.GetActor();
			ReplicatedRelativeData.bIsAnchored = true;

			// 2. [신규] 서버에 보고! (이게 없어서 그동안 고장났던 것임)
			Server_SetAnchoring(Hit.GetActor());
		}

		CurrentFloorNormal = Hit.Normal;

		AActor* NewFloor = Hit.GetActor();

		if (GetAttachParentActor() != NewFloor)
		{
			AttachToActor(NewFloor, FAttachmentTransformRules::KeepWorldTransform);
			GetCharacterMovement()->SetMovementMode(MOVE_Custom);
			ReplicatedRelativeData.BaseActor = NewFloor;
			ReplicatedRelativeData.bIsAnchored = true;
		}

		// [중요] 바닥의 기울기를 저장하여 Tick 함수로 전달 (이동 방향 계산용)
		CurrentFloorNormal = Hit.Normal;

		// [높이 보정] Hit.ImpactPoint + Normal * Height 방식 (과거에 잘 작동했던 방식)
		float TargetHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + FloorHeightOffset;
		FVector TargetLoc = Hit.ImpactPoint + (Hit.Normal * TargetHeight);

		FVector NewLoc = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, AlignSpeed);
		SetActorLocation(NewLoc);

		// 회전 정렬
		FRotator CurrentRot = GetActorRotation();
		FRotator TargetRot = FRotationMatrix::MakeFromZX(GravityUpDir, GetActorForwardVector()).Rotator();
		FQuat NewQuat = FMath::QInterpTo(CurrentRot.Quaternion(), TargetRot.Quaternion(), DeltaTime, AlignSpeed);
		SetActorRotation(NewQuat);
	}
	else
	{
		// 바닥이 없을 때 처리
		if (ReplicatedRelativeData.bIsAnchored)
		{
			// 바닥을 잃었으면 서버에 해제 요청
			Server_SetAnchoring(nullptr);

			ReplicatedRelativeData.BaseActor = nullptr;
			ReplicatedRelativeData.bIsAnchored = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}

		// 바닥 못 찾음
		CurrentFloorNormal = FVector::ZeroVector;

		// 강제 하강 (Gravity)
		FVector FallVector = DownDir * FlyModeMaxSpeed * DeltaTime;
		FHitResult FallHit;
		GetCharacterMovement()->SafeMoveUpdatedComponent(FallVector, GetActorRotation(), true, FallHit);
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

void APSJ_Character::StartDisembarkState()
{
	bJustDisembarked = true;
	DisembarkGraceTimer = 0.2f; // 0.2초간 유예 (우주선 속도에 따라 조절 가능)
	LastFloorActor = nullptr;
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

	// 이제 서버도 BaseActor가 누군지 알기 때문에 이 조건문이 통과됩니다!
	// -> 서버 캐릭터도 회전하기 시작함.
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

// [PSJ_Character.cpp]

bool APSJ_Character::Server_SetAnchoring_Validate(AActor* NewBase)
{
	return true;
}

void APSJ_Character::Server_SetAnchoring_Implementation(AActor* NewBase)
{
	// 1. 데이터 갱신 (이제 서버도 BaseActor가 누군지 알게 됨)
	ReplicatedRelativeData.BaseActor = NewBase;
	ReplicatedRelativeData.bIsAnchored = (NewBase != nullptr);

	if (NewBase)
	{
		// 2. 서버 차원에서 물리적 부착 수행
		AttachToActor(NewBase, FAttachmentTransformRules::KeepWorldTransform);

		// 3. [핵심] 서버 물리 엔진의 간섭 차단
		// DisableMovement()는 쓰지 마세요. 대신 Custom 모드로 전환.
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);

		// 4. [핵심] "이제부터 위치 동기화는 RPC로 수동으로 할 테니, 엔진 너는 빠져"
		SetReplicateMovement(false);
	}
	else
	{
		// 부착 해제 시 복구
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(true);
	}
}