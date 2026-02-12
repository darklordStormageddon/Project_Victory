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

	// [필수 추가] 이 한 줄이 없으면 변수 동기화가 안 될 수 있습니다.
	bReplicates = true;
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

	// 1. 착지 시 기본 모드를 '걷기'가 아닌 '비행'으로 변경
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	// 2. 걷을 수 있는 경사각을 0으로 설정 (어떤 바닥도 걷는 바닥으로 인식 안 함)
	GetCharacterMovement()->SetWalkableFloorAngle(0.0f);

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

	// [수정 후] 조건을 제거하여 모든 클라이언트가 확실하게 받도록 변경
	DOREPLIFETIME(APSJ_Character, CurrentInputVector);
	DOREPLIFETIME(APSJ_Character, bIsSprinting);
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
		UpdateRepairLogic();
		UCharacterMovementComponent* CMC = GetCharacterMovement();

		// [기획 확정] Walking 절대 금지 로직
		// Walking이나 Falling이 감지되면 즉시 올바른 모드로 강제 복구
		if (CMC->MovementMode == MOVE_Walking || CMC->MovementMode == MOVE_Falling)
		{
			// 앵커링 상태면 Custom, 아니면 무조건 Flying
			if (ReplicatedRelativeData.bIsAnchored)
			{
				CMC->SetMovementMode(MOVE_Custom);
			}
			else
			{
				CMC->SetMovementMode(MOVE_Flying);
			}

			// 관성으로 인한 미끄러짐 방지
			CMC->Velocity = FVector::ZeroVector;
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

	FVector GravityUpDir = FVector::UpVector;
	if (GetAttachParentActor()) GravityUpDir = GetAttachParentActor()->GetActorUpVector();
	FVector DownDir = -GravityUpDir;

	float MyHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float TraceHalfHeight = MagBootsTraceHalfHeight;

	float HeightDiff = TraceHalfHeight - MyHalfHeight;
	FVector StartOffset = GravityUpDir * (HeightDiff + 0.1f);

	FVector Start = GetActorLocation() + StartOffset;

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

	bool bFoundValidFloor = false;
	bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, ShapeRotation, ECC_Spaceship_Floor, CapsuleShape, Params);

	if (bHit)
	{
		bFoundValidFloor = true;
	}
	else
	{
		bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, ShapeRotation, ECC_Visibility, CapsuleShape, Params);

		if (bHit && Hit.GetActor())
		{
			if (Hit.GetActor()->ActorHasTag(TEXT("Stairs")))
			{
				bFoundValidFloor = true;
			}
			else
			{
				bFoundValidFloor = false;
			}
		}
	}

	// [신규] 점프 로직 시작
	if (bIsJumping)
	{
		// 1. 자력 감속 (Deceleration) 적용 - CMC 모드 변경 없음
		CurrentVerticalSpeed -= JumpDeceleration * DeltaTime;

		// 2. Z축 이동 (바닥 Normal 기준)
		// 점프 중에는 CurrentFloorNormal을 쓰되, 없으면 GravityUpDir 사용
		FVector JumpUpDir = !CurrentFloorNormal.IsZero() ? CurrentFloorNormal : GravityUpDir;
		FVector JumpDelta = JumpUpDir * CurrentVerticalSpeed * DeltaTime;
		AddActorWorldOffset(JumpDelta, true);

		// 3. 회전 보정 (점프 중에도 발바닥 각도 유지)
		if (bFoundValidFloor)
		{
			FRotator CurrentRot = GetActorRotation();
			FRotator TargetRot = FRotationMatrix::MakeFromZX(Hit.Normal, GetActorForwardVector()).Rotator();
			SetActorRotation(FMath::QInterpTo(CurrentRot.Quaternion(), TargetRot.Quaternion(), DeltaTime, AlignSpeed));
			CurrentFloorNormal = Hit.Normal;
		}

		// 4. 착지 판정 (속도가 음수이고, 바닥이 가까울 때)
		if (CurrentVerticalSpeed <= 0.0f && bFoundValidFloor)
		{
			if (Hit.Distance <= FloorHeightOffset + 5.0f)
			{
				bIsJumping = false;
				CurrentVerticalSpeed = 0.0f;

				if (GetAttachParentActor() != Hit.GetActor())
				{
					AttachToActor(Hit.GetActor(), FAttachmentTransformRules::KeepWorldTransform);
					ReplicatedRelativeData.BaseActor = Hit.GetActor();
					ReplicatedRelativeData.bIsAnchored = true;
					Server_SetAnchoring(Hit.GetActor());
				}
			}
		}

		// 점프 중에는 기존 자석 부츠 로직을 수행하지 않고 리턴
		return;
	}

	if (bFoundValidFloor && Hit.GetActor())
	{
		if (GetAttachParentActor() != Hit.GetActor())
		{
			AttachToActor(Hit.GetActor(), FAttachmentTransformRules::KeepWorldTransform);
			GetCharacterMovement()->SetMovementMode(MOVE_Custom);
			ReplicatedRelativeData.BaseActor = Hit.GetActor();
			ReplicatedRelativeData.bIsAnchored = true;
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

		CurrentFloorNormal = Hit.Normal;

		float TargetHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + FloorHeightOffset;
		FVector TargetLoc = Hit.ImpactPoint + (Hit.Normal * TargetHeight);

		FVector NewLoc = FMath::VInterpTo(GetActorLocation(), TargetLoc, DeltaTime, AlignSpeed);
		SetActorLocation(NewLoc);

		FRotator CurrentRot = GetActorRotation();
		FRotator TargetRot = FRotationMatrix::MakeFromZX(GravityUpDir, GetActorForwardVector()).Rotator();
		FQuat NewQuat = FMath::QInterpTo(CurrentRot.Quaternion(), TargetRot.Quaternion(), DeltaTime, AlignSpeed);
		SetActorRotation(NewQuat);
	}
	else
	{
		if (ReplicatedRelativeData.bIsAnchored)
		{
			Server_SetAnchoring(nullptr);

			ReplicatedRelativeData.BaseActor = nullptr;
			ReplicatedRelativeData.bIsAnchored = false;
			GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		}

		CurrentFloorNormal = FVector::ZeroVector;

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

	// [1] 매핑 컨텍스트(IMC) 등록 로직
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// [2] 액션 바인딩 로직
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// (1) 이동 (Move)
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APSJ_Character::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APSJ_Character::StopMove);
		}

		// (2) 시점 회전 (Look)
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APSJ_Character::Look);
		}

		// (3) 상호작용 (Interact - 탑승하기)
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APSJ_Character::Interact);
		}

		// (4) 점프 (Jump)
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APSJ_Character::Input_Jump);
		}

		// 주의: 에디터의 BP_Character에서 SprintAction에 IA_Sprint를 꼭 넣어주세요!
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APSJ_Character::Input_SprintStart);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APSJ_Character::Input_SprintStop);
		}

		// [신규 추가] 강제 하차 (Force Eject - 마우스 우클릭 등)
		if (ForceEjectAction)
		{
			EnhancedInputComponent->BindAction(ForceEjectAction, ETriggerEvent::Started, this, &APSJ_Character::Input_ForceEject);
		}

		// [신규] 수리 (Left Mouse Button)
		if (RepairAction)
		{
			// 누르는 순간 -> bIsRepairingInputDown = true
			EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Started, this, &APSJ_Character::Input_StartRepair);
			// 떼는 순간 -> bIsRepairingInputDown = false
			EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Completed, this, &APSJ_Character::Input_StopRepair);
		}
	}
}

void APSJ_Character::SetCurrentSpaceship(APawn* NewSpaceship)
{
	CurrentSpaceship = NewSpaceship;
}


void APSJ_Character::Interact(const FInputActionValue& Value)
{
	if (!Controller) return;

	// 1. 시선 추적 (Line Trace) - "내 눈앞에 좌석이 있는가?"
	FVector TraceStart;
	FRotator TraceRot;

	if (FPSCamera)
	{
		TraceStart = FPSCamera->GetComponentLocation();
		TraceRot = FPSCamera->GetComponentRotation();
	}
	else
	{
		GetController()->GetPlayerViewPoint(TraceStart, TraceRot);
	}

	FVector TraceEnd = TraceStart + (TraceRot.Vector() * 300.0f); // 3m 거리 체크

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // 나는 무시

	// Trace 채널은 프로젝트 설정에 맞게 (Visibility or Interaction)
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// 디버그 라인 (테스트 후 주석 처리)
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 1.0f);

	if (bHit && HitResult.GetActor())
	{
		// 2. 콕핏(좌석)인지 확인
		if (APSJ_ShipCockpit* HitCockpit = Cast<APSJ_ShipCockpit>(HitResult.GetActor()))
		{
			// [핵심 해결] 콕핏에게 탑승 처리를 위임합니다.
			// 콕핏이 알아서 TargetPawn을 확인하고 Character의 RPC를 불러줍니다.
			HitCockpit->AttemptBoarding(this);
			return; // 탑승 시도했으면 함수 종료
		}
	}

	// 3. [예외 처리] 눈앞에 좌석은 없지만, 이미 우주선 내부에 탑승한 상태라면?
	// (이 부분은 기획 의도에 따라 남겨두거나 삭제하세요. 
	//  예: 우주선 안에서 허공에 대고 F 누르면 조종석으로 순간이동 시킬 것인가?)
	if (CurrentSpaceship)
	{
		// 만약 조종석을 직접 바라보지 않고도 탑승하게 하려면 이 로직 유지.
		// 하지만 멀티플레이어 환경에서 오작동 가능성이 있어 권장하진 않습니다.
		/*
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (APSJ_Spaceship* TargetShip = Cast<APSJ_Spaceship>(CurrentSpaceship))
			{
				Server_RequestBoarding(TargetShip);
			}
		}
		*/
	}
}

void APSJ_Character::Move(const FInputActionValue& Value)
{
	CurrentInputVector = Value.Get<FVector2D>();
	// 2. [추가] 서버한테도 알려줌!
	Server_SetInputVector(CurrentInputVector);

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
	// 2. [추가] 서버한테 멈췄다고 알려줌!
	Server_SetInputVector(FVector2D::ZeroVector);
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

void APSJ_Character::Input_Jump(const FInputActionValue& Value)
{
	// 앵커링 상태이고, 이미 점프 중이 아닐 때만 발동
	if (ReplicatedRelativeData.bIsAnchored && !bIsJumping)
	{
		bIsJumping = true;
		CurrentVerticalSpeed = JumpInitialSpeed;
		// 점프 즉시 바닥 부착 해제 처리나 Mode 변경을 하지 않음 (User Request)
		// 오직 bIsJumping 플래그로만 제어

		// 디버깅용 로그
		// UE_LOG(LogTemp, Log, TEXT("Jump Started! Speed: %f"), CurrentVerticalSpeed);
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

// 입력 처리 함수 (클라이언트)
void APSJ_Character::Input_ForceEject(const FInputActionValue& Value)
{
	// 내가 탑승 중이면 실행 불가 (내부에서 내리는 건 Interact 키로)
	if (CurrentSpaceship) return;

	FVector TraceStart;
	FRotator TraceRot;

	// 카메라 위치 기준
	if (FPSCamera)
	{
		TraceStart = FPSCamera->GetComponentLocation();
		TraceRot = FPSCamera->GetComponentRotation();
	}
	else
	{
		GetController()->GetPlayerViewPoint(TraceStart, TraceRot);
	}

	FVector TraceEnd = TraceStart + (TraceRot.Vector() * ForceEjectRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// [중요] 콕핏의 메쉬(Mesh)나 콜리전 박스가 'Visibility' 채널을 'Block' 하고 있어야 감지됩니다.
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

#if WITH_EDITOR
	// 디버그 라인 확인: 초록색이면 히트 성공, 빨간색이면 허공
	if (bHit) DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Green, false, 1.0f);
	else DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 1.0f);
#endif

	if (bHit && HitResult.GetActor())
	{
		// 콕핏 클래스로 캐스팅 (상속받은 모든 블루프린트 포함)
		if (APSJ_ShipCockpit* HitCockpit = Cast<APSJ_ShipCockpit>(HitResult.GetActor()))
		{
			// 서버에 요청
			Server_TryForceEject(HitCockpit);
		}
	}
}

// 3. 서버 RPC 구현
bool APSJ_Character::Server_TryForceEject_Validate(APSJ_ShipCockpit* TargetCockpit)
{
	if (TargetCockpit)
	{
		// 거리 검증 (약간의 여유 허용)
		float DistanceSq = FVector::DistSquared(GetActorLocation(), TargetCockpit->GetActorLocation());
		float AllowedRangeSq = FMath::Square(ForceEjectRange * 1.5f);
		if (DistanceSq > AllowedRangeSq)
		{
			return false;
		}
	}
	return true;
}

void APSJ_Character::Server_TryForceEject_Implementation(APSJ_ShipCockpit* TargetCockpit)
{
	if (TargetCockpit)
	{
		// 콕핏에게 하차 요청 전달
		TargetCockpit->ReceiveForceEjectRequest();
	}
}

bool APSJ_Character::Server_RequestPawnPossess_Validate(APawn* TargetPawn)
{
	return true;
}

void APSJ_Character::Server_RequestPawnPossess_Implementation(APawn* TargetPawn)
{
	if (!TargetPawn) return;

	// 컨트롤러 가져오기
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 1. 캐릭터 움직임 멈추기 (선택사항)
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}

		// 2. 패널(TargetPawn) 조종 시작!
		PC->Possess(TargetPawn);

		// (참고) 만약 패널 쪽에서 "탑승 완료되었습니다" 같은 처리가 필요하면
		// 여기서 TargetPawn->OnBoarded() 같은 함수를 호출해줄 수도 있습니다.
	}
}

void APSJ_Character::Client_RestoreInputRPC_Implementation()
{
	// 이 코드는 클라이언트 컴퓨터에서 실행됩니다.
	// 기존에 만들어둔 "입력 강제 복구 함수"를 실행하여 마우스/키보드를 활성화합니다.
	ForceInputRecovery();

	// 혹시 모를 안전장치: 엔진 표준 입력 재시작 함수도 같이 호출
	PawnClientRestart();

	// 로그로 확인
	// UE_LOG(LogTemp, Warning, TEXT("[RPC] Client Input Restored via Blueprint Request!"));
}

// =========================================================
// [신규] 수리 로직 구현부
// =========================================================

void APSJ_Character::Input_StartRepair(const FInputActionValue& Value)
{
	bIsRepairingInputDown = true;
}

void APSJ_Character::Input_StopRepair(const FInputActionValue& Value)
{
	bIsRepairingInputDown = false;

	// 버튼을 뗐으므로 즉시 수리 중단 요청
	if (ClientRepairTarget)
	{
		Server_StopRepair();
		ClientRepairTarget = nullptr;
	}
}

void APSJ_Character::UpdateRepairLogic()
{
	// 1. 버튼을 안 누르고 있으면 아무것도 안 함
	if (!bIsRepairingInputDown) return;

	// 2. 시선 트레이스 (RepairTraceLength 사용)
	FVector TraceStart;
	FRotator TraceRot;
	if (FPSCamera)
	{
		TraceStart = FPSCamera->GetComponentLocation();
		TraceRot = FPSCamera->GetComponentRotation();
	}
	else
	{
		GetController()->GetPlayerViewPoint(TraceStart, TraceRot);
	}

	FVector TraceEnd = TraceStart + (TraceRot.Vector() * RepairTraceLength);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// 가시성(Visibility) 채널로 체크
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// =========================================================
	// [신규] 비주얼 디버그 라인 그리기 (좌클릭 유지 시 보임)
	// =========================================================
	if (bHit)
	{
		// 충돌 지점까지 초록색 선
		DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Green, false, 0.1f, 0, 1.0f);
		// 충돌 위치에 점 찍기
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 0.1f);
	}
	else
	{
		// 허공에 빨간색 선
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
	}
	// =========================================================

	APSJ_ShipCockpit* HitCockpit = nullptr;
	if (bHit && HitResult.GetActor())
	{
		HitCockpit = Cast<APSJ_ShipCockpit>(HitResult.GetActor());
	}

	// 3. 수리 가능 여부 판단
	bool bCanRepair = false;

	if (HitCockpit)
	{
		// (1) 고장난 상태인가?
		if (HitCockpit->bIsMalfunctioning)
		{
			// (2) 거리가 가까운가? (RepairMaxDistance 체크)
			float Dist = FVector::Dist(GetActorLocation(), HitCockpit->GetActorLocation());
			if (Dist <= RepairMaxDistance)
			{
				bCanRepair = true;
			}
			else
			{
				// [선택 사항] 화면에 "너무 멀음!" 메시지 띄우기 가능
				// PrintString: Too Far to Repair!
			}
		}
	}

	// 4. 상태 변화 처리
	if (bCanRepair)
	{
		// 타겟이 바뀌었거나, 새로 수리를 시작하는 경우
		if (ClientRepairTarget != HitCockpit)
		{
			// 기존 타겟이 있었다면 중단
			if (ClientRepairTarget) Server_StopRepair();

			// 새 타겟 수리 시작
			Server_StartRepair(HitCockpit);
			ClientRepairTarget = HitCockpit;
		}
	}
	else
	{
		// 조준 실패, 거리 멀어짐, 혹은 고장 수리 완료됨 -> 수리 중단
		if (ClientRepairTarget != nullptr)
		{
			Server_StopRepair();
			ClientRepairTarget = nullptr;
		}
	}
}

// [서버] 수리 시작
bool APSJ_Character::Server_StartRepair_Validate(APSJ_ShipCockpit* TargetCockpit) { return true; }
void APSJ_Character::Server_StartRepair_Implementation(APSJ_ShipCockpit* TargetCockpit)
{
	if (TargetCockpit)
	{
		// 콕핏에 나를 등록 (수리 인원 +1)
		TargetCockpit->AddRepairer(this);

		// 서버도 내가 누굴 수리하는지 기억해둠 (나중에 끊길 때 대비)
		ServerRepairTarget = TargetCockpit;
	}
}

// [서버] 수리 중단
bool APSJ_Character::Server_StopRepair_Validate() { return true; }
void APSJ_Character::Server_StopRepair_Implementation()
{
	// 내가 기억하고 있는 콕핏에게서 나를 제거
	if (ServerRepairTarget)
	{
		ServerRepairTarget->RemoveRepairer(this);
		ServerRepairTarget = nullptr;
	}
}

// 2. 파일 맨 아래(혹은 편한 곳)에 구현부를 추가하세요.
void APSJ_Character::Input_SprintStart(const FInputActionValue& Value)
{
	bIsSprinting = true;
}

void APSJ_Character::Input_SprintStop(const FInputActionValue& Value)
{
	bIsSprinting = false;
}

// --- 입력 벡터 동기화 RPC ---
bool APSJ_Character::Server_SetInputVector_Validate(FVector2D NewInput)
{
	return true;
}

void APSJ_Character::Server_SetInputVector_Implementation(FVector2D NewInput)
{
	// 서버가 클라이언트의 입력값을 받아서 자기 변수에 업데이트
	CurrentInputVector = NewInput;
}

// --- 달리기 동기화 RPC ---
bool APSJ_Character::Server_SetSprinting_Validate(bool bNewSprinting)
{
	return true;
}

void APSJ_Character::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
}