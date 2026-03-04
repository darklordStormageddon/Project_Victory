#include "PSJ_Character.h"
#include "JHS/Interact/InteracterComponent.h"
#include "PSJ_Spaceship.h"
#include "PSJ/TaskChair.h"
#include "PSJ_ToolBase.h"
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

	bReplicates = true;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();


	if (!InteracterComponent)
	{
		InteracterComponent = FindComponentByClass<UInteracterComponent>();
	}

	if (HasAuthority())
	{
		AJHSGameMode* _outGameMode = nullptr;
		if (UStaticFunctionLibrary::TryGetGameMode(this, _outGameMode))
		{
			_outGameMode->StartGame(this);
		}
	}

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	GetCharacterMovement()->SetWalkableFloorAngle(0.0f);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = FlyModeMaxSpeed;

	FPSCamera = FindComponentByClass<UCameraComponent>();
	if (GetMesh())
	{
		DefaultMeshZ = GetMesh()->GetRelativeLocation().Z;
	}

	if (ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(false);
	}

	if (HasAuthority() && ToolClassToSpawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		EquippedTool = GetWorld()->SpawnActor<APSJ_ToolBase>(ToolClassToSpawn, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (EquippedTool)
		{
			EquippedTool->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Gun"));
		}
	}

}

ATaskChair* APSJ_Character::GetRepairTargetFromTrace()
{
	if (!FPSCamera) return nullptr;

	FVector TraceStart = FPSCamera->GetComponentLocation();
	FVector TraceEnd = TraceStart + (FPSCamera->GetForwardVector() * RepairTraceLength);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// 디버그 라인 (에디터 확인용)
	if (bHit)
	{
		DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Green, false, 0.1f, 0, 1.0f);
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 0.1f);
	}
	else
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
	}

	if (bHit && HitResult.GetActor())
	{
		ATaskChair* TargetChair = Cast<ATaskChair>(HitResult.GetActor());

		if (TargetChair && TargetChair->bIsMalfunctioning)
		{
			float Dist = FVector::Dist(GetActorLocation(), TargetChair->GetActorLocation());
			if (Dist <= RepairMaxDistance)
			{
				return TargetChair;
			}
		}
	}
	return nullptr;
}

void APSJ_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSJ_Character, ReplicatedRelativeData);

	DOREPLIFETIME(APSJ_Character, CurrentInputVector);
	DOREPLIFETIME(APSJ_Character, bIsSprinting);

	DOREPLIFETIME(APSJ_Character, bIsActivelyRepairing);

	DOREPLIFETIME(APSJ_Character, EquippedTool); 
}

void APSJ_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

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

	if (bJustDisembarked)
	{
		DisembarkGraceTimer -= DeltaTime;
		if (DisembarkGraceTimer <= 0.0f) bJustDisembarked = false;
	}

	if (IsLocallyControlled())
	{
		UpdateRepairLogic();
		UCharacterMovementComponent* CMC = GetCharacterMovement();

		if (CMC->MovementMode == MOVE_Walking || CMC->MovementMode == MOVE_Falling)
		{
			if (ReplicatedRelativeData.bIsAnchored) CMC->SetMovementMode(MOVE_Custom);
			else CMC->SetMovementMode(MOVE_Flying);
			CMC->Velocity = FVector::ZeroVector;
		}
	}

	if (!Controller || (IsLocallyControlled() && CurrentSpaceship)) return;

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

	if (IsLocallyControlled() || HasAuthority())
	{
		if (!CurrentInputVector.IsNearlyZero())
		{
			FVector LocalDir = FVector(CurrentInputVector.Y, CurrentInputVector.X, 0.0f);
			FVector WorldDir = GetActorQuat().RotateVector(LocalDir);

			if (ReplicatedRelativeData.bIsAnchored && !CurrentFloorNormal.IsZero())
			{
				WorldDir = FVector::VectorPlaneProject(WorldDir, CurrentFloorNormal).GetSafeNormal();
			}

			FVector MoveDelta = WorldDir * FlyModeMaxSpeed * DeltaTime;
			FHitResult MoveHit;

			GetCharacterMovement()->SafeMoveUpdatedComponent(MoveDelta, GetActorRotation(), true, MoveHit);

			if (MoveHit.IsValidBlockingHit())
			{
				FVector SlideVector = FVector::VectorPlaneProject(MoveDelta, MoveHit.Normal);
				GetCharacterMovement()->SafeMoveUpdatedComponent(SlideVector * (1.0f - MoveHit.Time), GetActorRotation(), true, MoveHit);
			}
		}

		UpdateMagBoots(DeltaTime);

		// [개선됨] 로컬 플레이어인 경우 서버로 위치/회전 전송 (입력 변화 및 시간 임계값 기준)
		if (IsLocallyControlled())
		{
			float CurrentTime = GetWorld()->GetTimeSeconds();
			FVector CurrentLoc = GetRootComponent()->GetRelativeLocation();
			FRotator CurrentRot = GetRootComponent()->GetRelativeRotation();

			// 1. 방향키 입력이 달라졌는가?
			bool bInputChanged = !CurrentInputVector.Equals(LastSentInputVector);
			// 2. 마우스를 돌려서 회전각이 일정 수준(예: 1도) 이상 변했는가?
			bool bRotationChanged = !CurrentRot.Equals(LastSentRelativeRotation, 1.0f);
			// 3. 강제 동기화 시간이 지났는가?
			bool bTimeThreshold = (CurrentTime - LastNetUpdateTime) >= MaxNetUpdateDelay;

			// 셋 중 하나라도 해당되면 서버(혹은 본인 변수)에 갱신!
			if (bInputChanged || bRotationChanged || bTimeThreshold)
			{
				if (!HasAuthority())
				{
					// 일반 클라이언트면 서버로 RPC 전송
					Server_UpdateRelativeTransform(CurrentLoc, CurrentRot);
				}
				else
				{
					// 호스트(서버 본인)면 남들이 볼 수 있게 변수 직접 갱신
					ReplicatedRelativeData.RelativeLocation = CurrentLoc;
					ReplicatedRelativeData.RelativeRotation = CurrentRot;
				}

				// 현재 상태 저장
				LastSentInputVector = CurrentInputVector;
				LastSentRelativeRotation = CurrentRot;
				LastNetUpdateTime = CurrentTime;
			}
		}
	}
	else
	{
		// 다른 클라이언트(Simulated Proxy)들의 움직임 보간 적용
		if (ReplicatedRelativeData.BaseActor)
		{
			FVector OldRelLocation = GetRootComponent()->GetRelativeLocation();
			FRotator OldRelRotation = GetRootComponent()->GetRelativeRotation();

			FVector TargetLoc = FVector(ReplicatedRelativeData.RelativeLocation);
			FRotator TargetRot = ReplicatedRelativeData.RelativeRotation;

			// [수정된 부분] 즉각적인 Set 대신 VInterpTo, RInterpTo를 사용해 부드럽게 위치 및 회전 보간
			// AlignSpeed를 활용해 목표 위치로 부드럽게 따라가도록 처리
			FVector NewLoc = FMath::VInterpTo(OldRelLocation, TargetLoc, DeltaTime, AlignSpeed);
			FRotator NewRot = FMath::RInterpTo(OldRelRotation, TargetRot, DeltaTime, AlignSpeed);

			SetActorRelativeLocation(NewLoc);
			SetActorRelativeRotation(NewRot);

			if (DeltaTime > KINDA_SMALL_NUMBER)
			{
				// [수정된 부분] 변경된 위치(NewLoc)를 바탕으로 속도 계산 
				FVector RelDelta = (NewLoc - OldRelLocation) / DeltaTime;
				GetCharacterMovement()->Velocity = RelDelta;
			}

			if (GetMesh())
			{
				GetMesh()->TickAnimation(DeltaTime, false);
				GetMesh()->RefreshBoneTransforms();
			}

			if (GetCharacterMovement()->MovementMode != MOVE_Custom)
			{
				GetCharacterMovement()->SetMovementMode(MOVE_Custom);
			}
		}
	}
}

void APSJ_Character::ForceExecuteMagBoots()
{
	// DeltaTime을 1.0f로 크게 주어 InterpTo(보간)를 무시하고 즉시 바닥에 밀착 및 각도 정렬되게 합니다.
	UpdateMagBoots(1.0f);
}

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	// 서버(HasAuthority)도 자석 부츠 연산을 똑같이 수행하도록 열어줌
	if (!IsLocallyControlled() && !HasAuthority()) return;

	FVector GravityUpDir = FVector::UpVector;
	AActor* AttachedActor = GetAttachParentActor();

	if (AttachedActor)
	{
		if (AttachedActor->ActorHasTag(TEXT("Stairs")))
		{
			AActor* ParentActor = AttachedActor->GetAttachParentActor();
			GravityUpDir = ParentActor ? ParentActor->GetActorUpVector() : FVector::UpVector;
		}
		else
		{
			GravityUpDir = AttachedActor->GetActorUpVector();
		}
	}


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

	// ==========================================
	// 수정된 충돌 검사 및 바닥 판별 로직 시작
	// ==========================================
	bool bFoundValidFloor = false;
	bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, ShapeRotation, ECC_Spaceship_Floor, CapsuleShape, Params);

	if (bHit && Hit.GetActor())
	{
		// 다른 캐릭터를 바닥으로 인식하는 것을 완벽히 차단
		if (!Hit.GetActor()->IsA(APSJ_Character::StaticClass()))
		{
			bFoundValidFloor = true;
		}
	}
	else
	{
		bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, ShapeRotation, ECC_Visibility, CapsuleShape, Params);

		if (bHit && Hit.GetActor())
		{
			// 1. 맞은 액터가 캐릭터인 경우 무조건 바닥 취급 안 함
			if (Hit.GetActor()->IsA(APSJ_Character::StaticClass()))
			{
				bFoundValidFloor = false;
			}
			// 2. 계단 태그가 있는 경우 바닥으로 인정
			else if (Hit.GetActor()->ActorHasTag(TEXT("Stairs")))
			{
				bFoundValidFloor = true;
			}
			// 3. 그 외의 오브젝트는 바닥으로 취급 안 함
			else
			{
				bFoundValidFloor = false;
			}
		}
	}
	// ==========================================
	// 수정된 충돌 검사 및 바닥 판별 로직 끝
	// ==========================================

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

		
		FVector ToTarget = TargetLoc - GetActorLocation();
		FVector HeightAdjustment = Hit.Normal * FVector::DotProduct(ToTarget, Hit.Normal);
		FVector CorrectedTargetLoc = GetActorLocation() + HeightAdjustment;

		FVector NewLoc = FMath::VInterpTo(GetActorLocation(), CorrectedTargetLoc, DeltaTime, AlignSpeed);
		SetActorLocation(NewLoc);

		FRotator CurrentRot = GetActorRotation();

		FVector FinalUpDir = GravityUpDir;
		AActor* FloorActor = Hit.GetActor();

		if (FloorActor->ActorHasTag(TEXT("Stairs")))
		{
			AActor* FloorParent = FloorActor->GetAttachParentActor();
			if (FloorParent)
			{
				FinalUpDir = FloorParent->GetActorUpVector();
			}
			else
			{
				FinalUpDir = FVector::UpVector;
			}
		}
		else
		{
			FinalUpDir = FloorActor->GetActorUpVector();
		}

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



void APSJ_Character::SetBaseActorData(AActor* NewBase)
{

	ReplicatedRelativeData.BaseActor = NewBase;
	ReplicatedRelativeData.bIsAnchored = (NewBase != nullptr);

	if (NewBase)
	{
		ReplicatedRelativeData.RelativeLocation = GetRootComponent()->GetRelativeLocation();
		ReplicatedRelativeData.RelativeRotation = GetRootComponent()->GetRelativeRotation();
	}

	CurrentInputVector = FVector2D::ZeroVector;
}

void APSJ_Character::ForceInputRecovery()
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

void APSJ_Character::StartDisembarkState()
{
	bJustDisembarked = true;
	DisembarkGraceTimer = 0.2f;
	LastFloorActor = nullptr;
}


void APSJ_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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


	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{

		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APSJ_Character::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APSJ_Character::StopMove);
		}


		if (WheelAction)
		{
			EnhancedInputComponent->BindAction(WheelAction, ETriggerEvent::Triggered, this, &APSJ_Character::Wheel);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APSJ_Character::Look);
		}

		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APSJ_Character::InteractEnter);
		}


		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APSJ_Character::Input_SprintStart);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APSJ_Character::Input_SprintStop);
		}


		if (ForceEjectAction)
		{
			EnhancedInputComponent->BindAction(ForceEjectAction, ETriggerEvent::Started, this, &APSJ_Character::Input_ForceEject);
		}


		if (RepairAction)
		{

			EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Started, this, &APSJ_Character::Input_StartRepair);
			EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Completed, this, &APSJ_Character::Input_StopRepair);
		}
	}
}

void APSJ_Character::SetCurrentSpaceship(APawn* NewSpaceship)
{
	CurrentSpaceship = NewSpaceship;
}


void APSJ_Character::InteractEnter(const FInputActionValue& Value)
{
	if (!Controller) return;

	if (InteracterComponent)
	{
		// 1. TryInteractInput에서 반환받을 상태값 변수 선언
		bool bOutIsInterrupt = false;
		bool bOutIsInteractEnter = false;

		// 2. 변수를 인자로 넣어서 함수 호출
		bool bSuccess = InteracterComponent->TryInteractInput(bOutIsInterrupt, bOutIsInteractEnter);

		// 필요하다면 bSuccess, bOutIsInterrupt, bOutIsInteractEnter 값에 따라 
		// 추가적인 캐릭터 로직(예: 애니메이션 재생 등)을 작성할 수 있습니다.
		if (bSuccess)
		{
			// 상호작용 성공 시 처리
		}
	}
}


bool APSJ_Character::TryUnboard()
{
	bool _isInterrupt = false;
	bool _isInteractEnter = false;
	bool _isSuccess = InteracterComponent->TryInteractInput( _isInterrupt, _isInteractEnter);
	if (_isSuccess && !_isInteractEnter)
		return true;
	return false;

}

void APSJ_Character::OnShootAnimFinished()
{
	bIsPlayingShootAnim = false;
}


void APSJ_Character::Move(const FInputActionValue& Value)
{

	if (bIsActivelyRepairing) return;

	FVector2D NewInput = Value.Get<FVector2D>();

	// 입력값이 이전과 다를 때만 서버로 전송하여 네트워크 부하 최소화
	if (!CurrentInputVector.Equals(NewInput, 0.01f))
	{
		CurrentInputVector = NewInput;
		Server_SetInputVector(CurrentInputVector);
	}
}

void APSJ_Character::StopMove(const FInputActionValue& Value)
{
	CurrentInputVector = FVector2D::ZeroVector;
	Server_SetInputVector(FVector2D::ZeroVector);
}

void APSJ_Character::Look(const FInputActionValue& Value)
{

	if (bIsPlayingShootAnim) return;

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

void APSJ_Character::Wheel(const FInputActionValue& Value)
{
	float WheelValue = Value.Get<float>();

	if (WheelValue != 0.0f)
	{

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

bool APSJ_Character::Server_RequestBoarding_Validate(ATaskPawnBase* TaskPawn)
{
	return TaskPawn != nullptr;
}

void APSJ_Character::Server_RequestBoarding_Implementation(ATaskPawnBase* TaskPawn)
{
	if (!TaskPawn) return;

	if (TaskPawn->CurrentPilot != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Seat Steal Blocked: The pawn is already occupied."));
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		TaskPawn->SetPilot(this);
		PC->Possess(TaskPawn);
		// 수정: Client_BoardingSuccess에 본인(Character)을 인자로 전달
		TaskPawn->Client_BoardingSuccess(this);
	}
}

void APSJ_Character::ForceClearAnchoring()
{
	ReplicatedRelativeData.BaseActor = nullptr;
	ReplicatedRelativeData.bIsAnchored = false;
	CurrentInputVector = FVector2D::ZeroVector;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentSpaceship = nullptr;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	SetReplicateMovement(true);
}

void APSJ_Character::Client_ForceCleanupImmediate()
{
	ReplicatedRelativeData.BaseActor = nullptr;
	ReplicatedRelativeData.bIsAnchored = false;

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CurrentSpaceship = nullptr;

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

		ForceInputRecovery();

	}
}

void APSJ_Character::Client_LateInputRestore()
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PawnClientRestart();
		ForceInputRecovery();
	}
}


bool APSJ_Character::Server_SetAnchoring_Validate(AActor* NewBase)
{
	return true;
}

void APSJ_Character::Server_SetAnchoring_Implementation(AActor* NewBase)
{
	ReplicatedRelativeData.BaseActor = NewBase;
	ReplicatedRelativeData.bIsAnchored = (NewBase != nullptr);

	if (NewBase)
	{
		AttachToActor(NewBase, FAttachmentTransformRules::KeepWorldTransform);

		GetCharacterMovement()->SetMovementMode(MOVE_Custom);


	}
	else
	{

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

	}
}

void APSJ_Character::Input_ForceEject(const FInputActionValue& Value)
{
	// 1. 우주선 탑승 중이거나 이미 애니메이션 재생 중이면 차단 (유지)
	if (CurrentSpaceship || bIsPlayingShootAnim) return;

	// 2. 도구의 타이머(쿨다운)가 동작 중이면 우클릭 기능 완전히 차단 (유지)
	if (EquippedTool && EquippedTool->bIsOnCooldown)
	{
		return;
	}

	// 3. 애니메이션 재생 요청 (기존 재생 로직을 지우고, 서버에 재생 요청을 보냅니다)
	if (ShootMontage && IsLocallyControlled())
	{
		Server_PlayShootMontage();
	}

	// ---------------------------------------------------------
	// 아래의 트레이스(Sweep) 및 의자 판별 로직은 기존 그대로 유지합니다.
	// ---------------------------------------------------------

	FVector StartLoc = GetActorLocation() + GetActorRotation().RotateVector(ForceEjectSphereOffset);
	FVector EndLoc = StartLoc + (GetActorForwardVector() * ForceEjectRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ForceEjectSphereRadius);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		StartLoc,
		EndLoc,
		FQuat::Identity,
		ECC_Visibility,
		SphereShape,
		QueryParams
	);

#if WITH_EDITOR
	FVector TraceVec = EndLoc - StartLoc;
	float TraceLen = TraceVec.Size();
	FVector CenterLoc = StartLoc + TraceVec * 0.5f;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();

	DrawDebugCapsule(GetWorld(), CenterLoc, TraceLen * 0.5f + ForceEjectSphereRadius, ForceEjectSphereRadius, CapsuleRot, bHit ? FColor::Green : FColor::Red, false, 2.0f);

	if (bHit)
	{
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 2.0f);
	}
#endif

	ATaskChair* TargetChair = nullptr;

	if (bHit && HitResult.GetActor())
	{
		TargetChair = Cast<ATaskChair>(HitResult.GetActor());
	}

	// 의자 강제 사출 서버 요청 (유지)
	Server_TryForceEject(TargetChair);
}

void APSJ_Character::Server_PlayShootMontage_Implementation()
{
	// 서버가 모든 클라이언트에게 멀티캐스트 함수를 실행하라고 명령합니다.
	Multicast_PlayShootMontage();
}

bool APSJ_Character::Server_PlayShootMontage_Validate()
{
	return true;
}

void APSJ_Character::Multicast_PlayShootMontage_Implementation()
{
	// 기존 Input_ForceEject에 있던 '애니메이션 재생 및 입력 제한' 로직이 이쪽으로 이사왔습니다.
	if (ShootMontage)
	{
		float Duration = PlayAnimMontage(ShootMontage);
		if (Duration > 0.0f)
		{
			bIsPlayingShootAnim = true;

			// 애니메이션 길이만큼 대기 후 OnShootAnimFinished 호출
			GetWorld()->GetTimerManager().SetTimer(
				ShootAnimTimerHandle,
				this,
				&APSJ_Character::OnShootAnimFinished,
				Duration,
				false
			);

			// 이동 중지 처리
			// (주의: Server_SetInputVector는 해당 캐릭터를 조종하는(LocallyControlled) 플레이어만 서버로 요청할 수 있으므로 조건문을 걸어줍니다)
			if (IsLocallyControlled())
			{
				CurrentInputVector = FVector2D::ZeroVector;
				Server_SetInputVector(FVector2D::ZeroVector);
			}

			// 캐릭터 속도를 0으로 만드는 것은 모든 플레이어 화면에서 똑같이 실행되어 연출을 맞춥니다.
			if (GetCharacterMovement())
			{
				GetCharacterMovement()->Velocity = FVector::ZeroVector;
			}
		}
	}
}

bool APSJ_Character::Server_TryForceEject_Validate(ATaskChair* TargetChair)
{
	if (TargetChair)
	{
		float DistanceSq = FVector::DistSquared(GetActorLocation(), TargetChair->GetActorLocation());

		float MaxAllowedDistance = ForceEjectRange + ForceEjectSphereRadius + ForceEjectSphereOffset.Size() + 200.0f;
		float AllowedRangeSq = FMath::Square(MaxAllowedDistance);

		if (DistanceSq > AllowedRangeSq)
		{
			return false; 
		}
	}
	return true;
}

void APSJ_Character::Server_TryForceEject_Implementation(ATaskChair* TargetChair)
{
	if (EquippedTool)
	{
		if (EquippedTool->bIsOnCooldown)
		{
			EquippedTool->Multicast_PlayEjectEffect(false, this);
			return;
		}
		else
		{
			EquippedTool->Multicast_PlayEjectEffect(true, this);
			EquippedTool->StartCooldownTimer();

			if (TargetChair) {
				TargetChair->ReceiveForceEjectRequest();
			}
		}
	}
}

bool APSJ_Character::Server_RequestPawnPossess_Validate(APawn* TargetPawn)
{
	return true;
}

void APSJ_Character::Server_RequestPawnPossess_Implementation(APawn* TargetPawn)
{
	if (!TargetPawn) return;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}

		PC->Possess(TargetPawn);

	}
}

void APSJ_Character::Client_RestoreInputRPC_Implementation()
{

	ForceInputRecovery();

	PawnClientRestart();

}


void APSJ_Character::Input_StartRepair(const FInputActionValue& Value)
{
	bIsRepairingInputDown = true;
}

void APSJ_Character::Input_StopRepair(const FInputActionValue& Value)
{
	bIsRepairingInputDown = false;
	bIsActivelyRepairing = false;

	if (ClientRepairTarget)
	{
		Server_StopRepair();
		ClientRepairTarget = nullptr;
	}
}

void APSJ_Character::UpdateRepairLogic()
{
	// 좌클릭을 누르고 있지 않으면 무시
	if (!bIsRepairingInputDown) return;

	// 누르고 있는 동안 매 프레임 트레이스 발사
	ATaskChair* TargetChair = GetRepairTargetFromTrace();

	// 시야에 조건이 맞는 대상(고장난 의자 + 사거리 내)이 있을 때
	if (TargetChair)
	{
		if (!bIsActivelyRepairing)
		{
			// 이동 멈춤 처리
			CurrentInputVector = FVector2D::ZeroVector;
			Server_SetInputVector(FVector2D::ZeroVector);
			GetCharacterMovement()->Velocity = FVector::ZeroVector;
		}

		bIsActivelyRepairing = true;

		// 타겟이 새로 잡혔거나 다른 의자로 바뀌었을 때 서버 연동
		if (ClientRepairTarget != TargetChair)
		{
			if (ClientRepairTarget) Server_StopRepair();
			Server_StartRepair(TargetChair);
			ClientRepairTarget = TargetChair;
		}
	}
	else
	{
		// 마우스를 누르고 있지만 허공을 보거나, 대상이 고쳐졌거나, 사거리 밖으로 벗어났을 때
		bIsActivelyRepairing = false;

		if (ClientRepairTarget)
		{
			Server_StopRepair();
			ClientRepairTarget = nullptr;
		}
	}
}


bool APSJ_Character::Server_StartRepair_Validate(ATaskChair* TargetChair) { return true; }
void APSJ_Character::Server_StartRepair_Implementation(ATaskChair* TargetChair)
{
	if (TargetChair)
	{

		TargetChair->AddRepairer(this);


		ServerRepairTarget = TargetChair;
	}

	bIsActivelyRepairing = true;
}


bool APSJ_Character::Server_StopRepair_Validate() { return true; }
void APSJ_Character::Server_StopRepair_Implementation()
{

	if (ServerRepairTarget)
	{
		ServerRepairTarget->RemoveRepairer(this);
		ServerRepairTarget = nullptr;
	}

	bIsActivelyRepairing = false;
}


void APSJ_Character::Input_SprintStart(const FInputActionValue& Value)
{
	bIsSprinting = true;
}

void APSJ_Character::Input_SprintStop(const FInputActionValue& Value)
{
	bIsSprinting = false;
}

bool APSJ_Character::Server_SetInputVector_Validate(FVector2D NewInput)
{
	return true;
}

void APSJ_Character::Server_SetInputVector_Implementation(FVector2D NewInput)
{
	CurrentInputVector = NewInput;
}

bool APSJ_Character::Server_SetSprinting_Validate(bool bNewSprinting)
{
	return true;
}

void APSJ_Character::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
}

void APSJ_Character::TeleportToSpaceship(const FVector& DestLocation, const FRotator& DestRotation)
{
	if (!HasAuthority()) return;

	ForceClearAnchoring();

	SetActorLocationAndRotation(DestLocation, DestRotation, false, nullptr, ETeleportType::TeleportPhysics);

	Client_TeleportAndReset(DestLocation, DestRotation);
}

void APSJ_Character::Client_TeleportAndReset_Implementation(const FVector& DestLocation, const FRotator& DestRotation)
{
	Client_ForceCleanupImmediate();

	SetActorLocationAndRotation(DestLocation, DestRotation, false, nullptr, ETeleportType::TeleportPhysics);
}