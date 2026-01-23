#include "PSJ_Character.h"
#include "PSJ_Spaceship.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
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
	// [추가] 시작하자마자 앵커링 데이터가 있다면 즉시 적용 (딜레이 방지)
	// 레벨 로딩 직후나 스폰 직후의 미끄러짐 방지
	if (ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		SetReplicateMovement(false); // 즉시 끄기
	}
}

void APSJ_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSJ_Character, ReplicatedRelativeData);
}

// [핵심] 하차 후 이동 불가 해결을 위한 상태 초기화
// 빙의(Possess)되는 순간 모든 이동 제한을 풀고 초기화합니다.
void APSJ_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 만약 이미 우주선에 잘 붙어있는 상태라면? -> 리셋 금지! 유지!
	if (ReplicatedRelativeData.BaseActor && ReplicatedRelativeData.bIsAnchored)
	{
		// 안전장치: 확실하게 상태만 다시 강제 (떼지는 않음)
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);
		SetReplicateMovement(false); // 엔진 간섭 차단 유지

		// 입력값만 초기화
		CurrentInputVector = FVector2D::ZeroVector;
	}
	else
	{
		// 붙어있는 게 없다면 그때 초기화 (기존 로직)
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

	// 조종 중인지 확인 (조종 중이면 캐릭터 로직 정지)
	if (Controller && IsLocallyControlled())
	{
		if (CurrentSpaceship)
		{
			// PossessedBy가 정상 작동했다면 여기 로직은 사실상 패스됨
		}
	}

	// 1. 앵커링(부착) 관리
	AActor* ParentActor = GetAttachParentActor();
	bool bShouldBeAttached = (ReplicatedRelativeData.BaseActor != nullptr);

	// [상태 전환: 부착 시작]
	if (bShouldBeAttached && ParentActor != ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);

		// [미끄러짐 해결 핵심] 엔진의 위치 동기화를 끕니다.
		// 우주선과 함께 움직이는 건 Attach가 담당하고,
		// 내부 이동 동기화는 우리가 만든 ReplicatedRelativeData가 담당합니다.
		SetReplicateMovement(false);
	}
	// [상태 전환: 부착 해제]
	else if (!bShouldBeAttached && ParentActor)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GetCharacterMovement()->Velocity = FVector::ZeroVector;

		// 우주선 밖에서는 엔진의 기본 동기화를 사용
		SetReplicateMovement(true);
	}

	// 2. 이동 로직
	if (GetAttachParentActor())
	{
		// [A] 내가 조종하는 캐릭터 (Autonomous + Server Host)
		if (IsLocallyControlled())
		{
			if (!CurrentInputVector.IsNearlyZero())
			{
				FVector LocalDir = FVector(CurrentInputVector.Y, CurrentInputVector.X, 0.0f);
				FVector DesiredMove = LocalDir * FlyModeMaxSpeed * DeltaTime;

				FHitResult Hit;
				AddActorLocalOffset(DesiredMove, true, &Hit);

				if (Hit.IsValidBlockingHit())
				{
					// [계단/경사면 오르기]
					FVector RealStepUp = GetActorUpVector() * 45.0f;
					FVector SavedLocation = GetActorLocation();

					FHitResult StepHit;
					AddActorWorldOffset(RealStepUp, true, &StepHit); // 들어올리기

					if (!StepHit.bBlockingHit)
					{
						AddActorLocalOffset(DesiredMove, true, &StepHit); // 전진
						if (!StepHit.bBlockingHit)
						{
							AddActorWorldOffset(-RealStepUp, true, &StepHit); // 내리기
						}
						else
						{
							SetActorLocation(SavedLocation);
							FVector SlideVector = FVector::VectorPlaneProject(DesiredMove, Hit.Normal);
							AddActorLocalOffset(SlideVector, true);
						}
					}
					else
					{
						SetActorLocation(SavedLocation);
						FVector SlideVector = FVector::VectorPlaneProject(DesiredMove, Hit.Normal);
						AddActorLocalOffset(SlideVector, true);
					}
				}
			}

			// 가짜 속도 주입 (애니메이션용)
			if (DeltaTime > 0.0f)
			{
				FVector TargetVel = CurrentInputVector.IsNearlyZero() ? FVector::ZeroVector : (GetActorForwardVector() * CurrentInputVector.Y + GetActorRightVector() * CurrentInputVector.X) * FlyModeMaxSpeed;
				GetCharacterMovement()->Velocity = TargetVel;
			}

			UpdateMagBoots(DeltaTime);

			// [중요] 위치 업데이트 (RPC + 로컬 변수)
			if (!HasAuthority())
			{
				Server_UpdateRelativeTransform(GetRootComponent()->GetRelativeLocation(), GetRootComponent()->GetRelativeRotation());
			}
			else
			{
				// 서버장인 경우 직접 갱신
				ReplicatedRelativeData.RelativeLocation = GetRootComponent()->GetRelativeLocation();
				ReplicatedRelativeData.RelativeRotation = GetRootComponent()->GetRelativeRotation();
			}
		}
		// [B] 남의 캐릭터 (Simulated Proxy)
		else
		{
			// 남의 캐릭터는 서버가 준 좌표로 '즉시' 이동 (보간 제거로 렉 방지)
			SetActorRelativeLocation(ReplicatedRelativeData.RelativeLocation);
			SetActorRelativeRotation(ReplicatedRelativeData.RelativeRotation);

			// 필요하다면 여기서 애니메이션용 Velocity 계산 추가 가능
		}
	}
	else
	{
		// 부착되지 않았을 때 (공중/우주선 밖)
		UpdateMagBoots(DeltaTime);

		// 비행 모드 이동
		if (IsLocallyControlled() && !CurrentInputVector.IsNearlyZero())
		{
			FVector WorldDir = GetActorForwardVector() * CurrentInputVector.Y + GetActorRightVector() * CurrentInputVector.X;
			AddMovementInput(WorldDir);
		}
	}
}

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	// [중요] 남의 캐릭터는 물리 보정을 하지 않음 (주인이 보낸 위치를 100% 신뢰)
	// 이것이 미세 떨림과 이중 보정을 막는 핵심입니다.
	if (!IsLocallyControlled() && GetAttachParentActor())
	{
		return;
	}

	FHitResult FinalHit;
	bool bFoundValidHit = false;
	FVector Start = GetActorLocation();

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

	bool bHit = GetWorld()->SweepMultiByChannel(HitResults, Start, End, FQuat::Identity, ECC_GameTraceChannel5, SphereShape, Params);

	if (bHit)
	{
		for (const FHitResult& Result : HitResults)
		{
			if (LastFloorActor && Result.GetActor() == LastFloorActor)
			{
				FinalHit = Result;
				bFoundValidHit = true;
				break;
			}
		}
		if (!bFoundValidHit && HitResults.Num() > 0)
		{
			FinalHit = HitResults[0];
			bFoundValidHit = true;
		}
	}

	AActor* NewFloorActor = bFoundValidHit ? FinalHit.GetActor() : nullptr;

	// [서버 로직]
	if (HasAuthority())
	{
		bool bUpdateData = IsLocallyControlled();
		if (!bUpdateData && bFoundValidHit && NewFloorActor)
		{
			bUpdateData = true;
		}

		if (bUpdateData)
		{
			if (bFoundValidHit && NewFloorActor)
			{
				if (ReplicatedRelativeData.BaseActor != NewFloorActor)
				{
					ReplicatedRelativeData.BaseActor = NewFloorActor;
					ReplicatedRelativeData.bIsAnchored = true;
				}
			}
			else
			{
				if (IsLocallyControlled())
				{
					ReplicatedRelativeData.BaseActor = nullptr;
					ReplicatedRelativeData.bIsAnchored = false;
				}
			}
		}
	}

	// [높이 보정]
	if (bFoundValidHit && NewFloorActor)
	{
		LastFloorActor = NewFloorActor;
		CurrentFloorNormal = NewFloorActor->GetActorUpVector();

		if (GetAttachParentActor())
		{
			float TargetHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			float ActualDistance = FVector::DotProduct(FinalHit.ImpactPoint - GetActorLocation(), TraceDir);
			if (ActualDistance <= 0.0f) ActualDistance = FinalHit.Distance + MagBootsTraceRadius;

			float HeightError = ActualDistance - TargetHeight;

			// 떨림 방지 (오차 1.0f)
			if (FMath::Abs(HeightError) > 1.0f)
			{
				float InterpSpeed = (HeightError > 0) ? 20.0f : 50.0f;
				float MoveZ = FMath::FInterpTo(0.0f, -HeightError, DeltaTime, InterpSpeed);
				AddActorWorldOffset(TraceDir * -MoveZ, false);
			}

			FRotator CurrentRelRot = GetRootComponent()->GetRelativeRotation();
			FRotator TargetRelRot = FRotator(0.0f, CurrentRelRot.Yaw, 0.0f);
			SetActorRelativeRotation(FMath::RInterpTo(CurrentRelRot, TargetRelRot, DeltaTime, AlignSpeed));
		}
	}
}

void APSJ_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext) Subsystem->AddMappingContext(DefaultMappingContext, 0);
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