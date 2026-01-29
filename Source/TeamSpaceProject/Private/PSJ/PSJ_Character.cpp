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
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();

	AJHSGameMode* _outGameMode = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
		return;

	_outGameMode->StartGame();

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

	// -------------------------------------------------------------------------
	// 1. 디버그 메시지 출력 (로컬 컨트롤러인 경우만)
	// -------------------------------------------------------------------------
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

	// -------------------------------------------------------------------------
	// 2. 조종 중 예외 처리
	// -------------------------------------------------------------------------
	if (Controller && IsLocallyControlled() && CurrentSpaceship) return;

	// -------------------------------------------------------------------------
	// 3. 앵커링(부착) 상태 관리
	// -------------------------------------------------------------------------
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

	// -------------------------------------------------------------------------
	// 4. 이동 및 벽 슬라이딩 로직 (핵심 수정 적용)
	// -------------------------------------------------------------------------
	if (GetAttachParentActor())
	{
		if (IsLocallyControlled())
		{
			if (!CurrentInputVector.IsNearlyZero())
			{
				// A. 로컬 입력 -> 월드 이동 벡터 변환
				FVector LocalDir = FVector(CurrentInputVector.Y, CurrentInputVector.X, 0.0f);
				FVector WorldDir = GetActorQuat().RotateVector(LocalDir);
				FVector RemainingMove = WorldDir * FlyModeMaxSpeed * DeltaTime;

				// B. 수동 벽 감지 및 슬라이딩 처리 (최대 3회 반사 - 구석 처리용)
				int32 MaxIterations = 3;
				for (int32 i = 0; i < MaxIterations; i++)
				{
					if (RemainingMove.IsNearlyZero()) break;

					FHitResult Hit;
					FCollisionQueryParams Params;
					Params.AddIgnoredActor(this); // 나 자신은 무시

					// [중요 수정] BaseActor(우주선) 무시 코드 삭제됨
					// 우주선의 자식 컴포넌트인 '벽'을 감지해야 하므로 우주선을 Ignore하면 안 됩니다.
					// 대신 아래에서 ObjectType으로 벽만 골라냅니다.

					FVector Start = GetActorLocation();
					FVector End = Start + RemainingMove;

					// 캡슐 크기를 아주 미세하게 줄여서 판정 (끼임 방지)
					FCollisionShape Shape = GetCapsuleComponent()->GetCollisionShape();
					Shape.Capsule.Radius *= 1.05f;
					Shape.Capsule.HalfHeight *= 1.0f;

					// [핵심] 채널 대신 '오브젝트 타입'으로 검사
					// WorldStatic(벽)만 찾고, Spaceship_Floor(바닥)나 WorldDynamic(본체)은 무시
					FCollisionObjectQueryParams ObjectParams;
					ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);

					bool bHit = GetWorld()->SweepSingleByObjectType(
						Hit, Start, End, GetActorQuat(), ObjectParams, Shape, Params
					);

					// [디버그 드로잉]
					FVector DebugCenter = End;
					FColor DebugColor = bHit ? FColor::Red : FColor::Green;
					DrawDebugCapsule(GetWorld(), DebugCenter, Shape.Capsule.HalfHeight, Shape.Capsule.Radius, GetActorQuat(), DebugColor, false, -1.0f, 0, 1.0f);

					if (bHit)
					{
						// 벽 감지됨 -> 디버그 메시지 출력
						GEngine->AddOnScreenDebugMessage(555, 0.0f, FColor::Red,
							FString::Printf(TEXT("Wall Detected: %s"), *Hit.GetActor()->GetName()));

						// 1. 부딪힌 지점 앞까지만 이동 (0.01f 간격 유지)
						FVector MoveToHit = Hit.TraceStart - Start + (Hit.Normal * 0.01f);
						AddActorWorldOffset(MoveToHit, false);

						// 2. 남은 힘의 방향을 벽면(Normal)에 따라 꺾음 (미끄러짐 처리)
						FVector SlideVector = FVector::VectorPlaneProject(RemainingMove, Hit.Normal);

						// 3. 이동한 비율만큼 남은 거리 차감
						float TimeUsed = Hit.Time;
						RemainingMove = SlideVector * (1.0f - TimeUsed);
					}
					else
					{
						// 벽 없음 -> 남은 거리 이동
						AddActorWorldOffset(RemainingMove, false);
						break;
					}
				}
			}

			// 속도 갱신 (애니메이션용)
			if (DeltaTime > 0.0f)
			{
				FVector TargetVel = CurrentInputVector.IsNearlyZero() ? FVector::ZeroVector :
					(GetActorForwardVector() * CurrentInputVector.Y + GetActorRightVector() * CurrentInputVector.X) * FlyModeMaxSpeed;
				GetCharacterMovement()->Velocity = TargetVel;
			}

			UpdateMagBoots(DeltaTime);

			// 서버 동기화
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
			// Simulated Proxy
			SetActorRelativeLocation(ReplicatedRelativeData.RelativeLocation);
			SetActorRelativeRotation(ReplicatedRelativeData.RelativeRotation);
		}
	}
	else // 공중 상태
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

void APSJ_Character::StartDisembarkState()
{
	bJustDisembarked = true;
	DisembarkGraceTimer = 0.2f; // 0.2초간 유예 (우주선 속도에 따라 조절 가능)
	LastFloorActor = nullptr;
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

	// 하차 유예 타이머 업데이트
	float CurrentCheckDistance = CheckDistance;
	if (bJustDisembarked)
	{
		DisembarkGraceTimer -= DeltaTime;
		if (DisembarkGraceTimer <= 0.0f) bJustDisembarked = false;

		// [핵심] 하차 직후엔 트레이스 거리를 2배로 늘려 우주선 하강에 대비
		CurrentCheckDistance = CheckDistance * 2.0f;
	}

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

			if (FMath::Abs(HeightError) > 0.5f)
			{
				// [핵심] 하차 직후에는 보정 속도를 아주 높여(40.0f 이상) 즉시 안착시킴
				float InterpSpeed = bJustDisembarked ? 45.0f : 20.0f;
				float MoveZ = FMath::FInterpTo(0.0f, -HeightError, DeltaTime, InterpSpeed);

				// bSweep을 false로 하여 물리 엔진에 의한 '끼임(Stuck)' 현상 방지
				// 자석 장화 로직이 위치를 강제로 잡아주기 때문입니다.
				AddActorLocalOffset(FVector(0, 0, MoveZ), false);
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