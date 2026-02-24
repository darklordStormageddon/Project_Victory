#include "PSJ_Character.h"
#include "PSJ_Spaceship.h"
#include "PSJ_ShipCockpit.h"
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

	// [�ʼ� �߰�] �� �� ���� ������ ���� ����ȭ�� �� �� �� �ֽ��ϴ�.
	bReplicates = true;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		AJHSGameMode* _outGameMode = nullptr;
		if (UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
		{
			_outGameMode->StartGame(this);
		}
	}

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 1. ���� �� �⺻ ��带 '�ȱ�'�� �ƴ� '����'���� ����
	GetCharacterMovement()->DefaultLandMovementMode = MOVE_Flying;
	// 2. ���� �� �ִ� ��簢�� 0���� ���� (� �ٴڵ� �ȴ� �ٴ����� �ν� �� ��)
	GetCharacterMovement()->SetWalkableFloorAngle(0.0f);

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->MaxFlySpeed = FlyModeMaxSpeed;

	FPSCamera = FindComponentByClass<UCameraComponent>();
	if (GetMesh())
	{
		DefaultMeshZ = GetMesh()->GetRelativeLocation().Z;
	}

	// [�߰�] ��Ŀ�� ������ ��� ����
	if (ReplicatedRelativeData.BaseActor)
	{
		AttachToActor(ReplicatedRelativeData.BaseActor, FAttachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(false);
	}

	// [�ű�] �������� ���� �����ϰ� ���Ͽ� �����մϴ�.
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

void APSJ_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APSJ_Character, ReplicatedRelativeData);

	// [���� ��] ������ �����Ͽ� ��� Ŭ���̾�Ʈ�� Ȯ���ϰ� �޵��� ����
	DOREPLIFETIME(APSJ_Character, CurrentInputVector);
	DOREPLIFETIME(APSJ_Character, bIsSprinting);

	// [�߰�] ���� ���µ� ����ȭ!
	DOREPLIFETIME(APSJ_Character, bIsActivelyRepairing);

	DOREPLIFETIME(APSJ_Character, EquippedTool); // �߰�
}

void APSJ_Character::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// �̹� ���ּ��� �پ��ִ� ���¶�� ���� ����
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

	// 1. ���� ��Ʈ�ѷ� ���� ���� (����, �����Ʈ ��� ����)
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

	// 2. ���� ���� ó�� (��Ʈ�ѷ� ���ų�, �����̸鼭 ���ּ� ���� ���̸� ����)
	if (!Controller || (IsLocallyControlled() && CurrentSpaceship)) return;

	// 3. ��Ŀ��(����) ���� ����ȭ (�θ� ���� ���� ����)
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

	// 4. �̵� ���� ����
	if (IsLocallyControlled())
	{
		// [Local] ���� ���� �����ϴ� ���: �Է¿� ���� ������ ��ġ�� �ű�
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

			// ������ ��ġ�� �̵���Ű�� �ٽ� �Լ�
			GetCharacterMovement()->SafeMoveUpdatedComponent(MoveDelta, GetActorRotation(), true, MoveHit);

			if (MoveHit.IsValidBlockingHit())
			{
				FVector SlideVector = FVector::VectorPlaneProject(MoveDelta, MoveHit.Normal);
				GetCharacterMovement()->SafeMoveUpdatedComponent(SlideVector * (1.0f - MoveHit.Time), GetActorRotation(), true, MoveHit);
			}
		}

		// �ٴ� ���� �� ���� ����
		UpdateMagBoots(DeltaTime);

		// ������ ���� ���� ��� ��ǥ�� ���� (�� ���� ������ ���� �ٸ� Ŭ���� ReplicatedRelativeData�� ��)
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
		// [Simulated Proxy] ������ �ٸ� Ŭ���̾�Ʈ�� ���� �� ��: ���޹��� ��ǥ�� ���� ����
		if (ReplicatedRelativeData.BaseActor)
		{
			FVector OldRelLocation = GetRootComponent()->GetRelativeLocation();

			// ��� ��ǥ ����ȭ
			SetActorRelativeLocation(ReplicatedRelativeData.RelativeLocation);
			SetActorRelativeRotation(ReplicatedRelativeData.RelativeRotation);

			// �ӵ� ���� (�ִϸ��̼� �����)
			if (DeltaTime > KINDA_SMALL_NUMBER)
			{
				FVector RelDelta = (FVector(ReplicatedRelativeData.RelativeLocation) - OldRelLocation) / DeltaTime;
				GetCharacterMovement()->Velocity = RelDelta;
			}

			// ������ ����: �޽� ���� ���� (���� ȭ�� A-Pose ����)
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

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	if (!IsLocallyControlled()) return;

	FVector GravityUpDir = FVector::UpVector;
	AActor* AttachedActor = GetAttachParentActor();

	if (AttachedActor)
	{
		// ������ �ٴ��� '���(Stairs)' �±׸� ������ �ִ°�?
		if (AttachedActor->ActorHasTag(TEXT("Stairs")))
		{
			AActor* ParentActor = AttachedActor->GetAttachParentActor();
			// ����� ���� �θ�(���ּ� ��)�� �ִٸ� �� �θ��� ������ ������,
			// �θ� ���ٸ�(���忡 �ܵ����� ��ġ�� ����) ������ ���� ����(Z-Up)�� ����
			GravityUpDir = ParentActor ? ParentActor->GetActorUpVector() : FVector::UpVector;
		}
		else
		{
			// ����� �ƴ� �Ϲ� �ٴ�(���ּ� ��ü ��)�̸� �ش� �ٴ��� ������ �״�� ����
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

	// [�ű�] ���� ���� ����
	if (bIsJumping)
	{
		// 1. �ڷ� ���� (Deceleration) ���� - CMC ��� ���� ����
		CurrentVerticalSpeed -= JumpDeceleration * DeltaTime;

		// 2. Z�� �̵� (�ٴ� Normal ����)
		// ���� �߿��� CurrentFloorNormal�� ����, ������ GravityUpDir ���
		FVector JumpUpDir = !CurrentFloorNormal.IsZero() ? CurrentFloorNormal : GravityUpDir;
		FVector JumpDelta = JumpUpDir * CurrentVerticalSpeed * DeltaTime;
		AddActorWorldOffset(JumpDelta, true);

		// 3. ȸ�� ���� (���� �߿��� �߹ٴ� ���� ����)
		if (bFoundValidFloor)
		{
			FRotator CurrentRot = GetActorRotation();
			FVector JumpAlignUpDir = FVector::UpVector; // �⺻���� ���� ���� ����

			AActor* HitActor = Hit.GetActor();
			if (HitActor)
			{
				if (HitActor->ActorHasTag(TEXT("Stairs")))
				{
					AActor* HitParent = HitActor->GetAttachParentActor();

					// [����] �θ� APSJ_Spaceship(���ּ�)���� ��Ȯ�� ĳ�����Ͽ� Ȯ��
					APSJ_Spaceship* ParentShip = Cast<APSJ_Spaceship>(HitParent);

					// �θ� ���ּ��̸� ���ּ��� Z��, ���ּ��� �ƴϸ�(�θ� ���ų� �ٸ� ���͸�) ������ ���� ���� ����
					JumpAlignUpDir = ParentShip ? ParentShip->GetActorUpVector() : FVector::UpVector;
				}
				else
				{
					// �Ϲ� �ٴ�/���ּ� ��ü�� ���
					JumpAlignUpDir = HitActor->GetActorUpVector();
				}
			}

			//  Hit.Normal ��� ������ JumpAlignUpDir ���
			FRotator TargetRot = FRotationMatrix::MakeFromZX(JumpAlignUpDir, GetActorForwardVector()).Rotator();
			SetActorRotation(FMath::QInterpTo(CurrentRot.Quaternion(), TargetRot.Quaternion(), DeltaTime, AlignSpeed));

			//  ���� ƽ�� ���� �����ϴ� �븻���� JumpAlignUpDir ���
			CurrentFloorNormal = JumpAlignUpDir;
		}

		// 4. ���� ���� (�ӵ��� �����̰�, �ٴ��� ����� ��)
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

		// ���� �߿��� ���� �ڼ� ���� ������ �������� �ʰ� ����
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

		// 1. ��� ���� �ٴ��� �������� ��¥ ���� ����(UpVector)�� �ٽ� ����
		FVector FinalUpDir = GravityUpDir; // �ϴ� ���� �߷� ������ �⺻���� ��
		AActor* FloorActor = Hit.GetActor();

		if (FloorActor->ActorHasTag(TEXT("Stairs")))
		{
			AActor* FloorParent = FloorActor->GetAttachParentActor();
			if (FloorParent)
			{
				// ���ּ� ���ο� ���ӵ� ����̸� �θ�(���ּ�)�� UpVector�� ���� (ȸ���ϴ� ���ּ� ���� �Ϻ� ����)
				FinalUpDir = FloorParent->GetActorUpVector();
			}
			else
			{
				// �ֻ��� �θ� ����? = ���忡 ���׷��� ���� �ܵ� ��� -> ���� ������ Z-Up ����
				FinalUpDir = FVector::UpVector;
			}
		}
		else
		{
			// ����� �ƴ� �Ϲ� ���ּ� �ٴ��̸� �ش� ������ UpVector�� ����
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


// [�ű�] ���� ���� �Լ�
void APSJ_Character::SetBaseActorData(AActor* NewBase)
{
	ReplicatedRelativeData.BaseActor = NewBase;
	ReplicatedRelativeData.bIsAnchored = (NewBase != nullptr);

	// �Է� ���ʹ� �ʱ�ȭ�ϵ�, �̵� ��� ��ü�� ���� ����
	CurrentInputVector = FVector2D::ZeroVector;
}

// [�ű�] �Է� ���� ���� �Լ� (�ٽ� �ذ�å)
void APSJ_Character::ForceInputRecovery()
{
	// �� ��ǻ���� 0�� ��Ʈ�ѷ�(�÷��̾�)�� ã��
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		// 1. �Է� �ý���(Enhanced Input) ��������
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// ���� ����(���ּ� Ű ��) �����ϰ� �� Ű(WASD) �߰�
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}

		// 2. [�߿�] �������� "�� ��Ʈ�ѷ��� �Է��� �� ���Ͱ� �ްڴ�"�� ����
		// ���������� InputComponent�� �����ϰ� ��Ʈ�ѷ� ���ÿ� Ǫ���մϴ�.
		EnableInput(PC);

		// 3. [�߿�] Ű ���ε�(Jump, Move ��) ����
		if (InputComponent)
		{
			SetupPlayerInputComponent(InputComponent);
		}

		// 4. �Է� ��� ���� ���� (UI �ݱ� ����)
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		//UE_LOG(LogTemp, Warning, TEXT("[Debug] ForceInputRecovery: Input Forced & Context Added!"));
	}
}

void APSJ_Character::StartDisembarkState()
{
	bJustDisembarked = true;
	DisembarkGraceTimer = 0.2f; // 0.2�ʰ� ���� (���ּ� �ӵ��� ���� ���� ����)
	LastFloorActor = nullptr;
}


void APSJ_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// [1] ���� ���ؽ�Ʈ(IMC) ��� ����
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

	// [2] �׼� ���ε� ����
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// (1) �̵� (Move)
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APSJ_Character::Move);
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &APSJ_Character::StopMove);
		}

		// (2) ���� ȸ�� (Look)
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APSJ_Character::Look);
		}


		// (3) ��ȣ�ۿ� (Interact - ž���ϱ�)
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APSJ_Character::Interact);
		}

		// (4) ���� (Jump)
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APSJ_Character::Input_Jump);
		}

		// ����: �������� BP_Character���� SprintAction�� IA_Sprint�� �� �־��ּ���!
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APSJ_Character::Input_SprintStart);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APSJ_Character::Input_SprintStop);
		}

		// [�ű� �߰�] ���� ���� (Force Eject - ���콺 ��Ŭ�� ��)
		if (ForceEjectAction)
		{
			EnhancedInputComponent->BindAction(ForceEjectAction, ETriggerEvent::Started, this, &APSJ_Character::Input_ForceEject);
		}

		// [�ű�] ���� (Left Mouse Button)
		if (RepairAction)
		{
			// ������ ���� -> bIsRepairingInputDown = true
			EnhancedInputComponent->BindAction(RepairAction, ETriggerEvent::Started, this, &APSJ_Character::Input_StartRepair);
			// ���� ���� -> bIsRepairingInputDown = false
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

	// 1. �ü� ���� (Line Trace) - "�� ���տ� �¼��� �ִ°�?"
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

	FVector TraceEnd = TraceStart + (TraceRot.Vector() * 300.0f); // 3m �Ÿ� üũ

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // ���� ����

	// Trace ä���� ������Ʈ ������ �°� (Visibility or Interaction)
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	// ����� ���� (�׽�Ʈ �� �ּ� ó��)
	// DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 1.0f);

	if (bHit && HitResult.GetActor())
	{
		// 2. ����(�¼�)���� Ȯ��
		if (APSJ_ShipCockpit* HitCockpit = Cast<APSJ_ShipCockpit>(HitResult.GetActor()))
		{
			// [�ٽ� �ذ�] ���Ϳ��� ž�� ó���� �����մϴ�.
			// ������ �˾Ƽ� TargetPawn�� Ȯ���ϰ� Character�� RPC�� �ҷ��ݴϴ�.
			HitCockpit->AttemptBoarding(this);
			return; // ž�� �õ������� �Լ� ����
		}
	}

	// 3. [���� ó��] ���տ� �¼��� ������, �̹� ���ּ� ���ο� ž���� ���¶��?
	// (�� �κ��� ��ȹ �ǵ��� ���� ���ܵΰų� �����ϼ���. 
	//  ��: ���ּ� �ȿ��� ����� ��� F ������ ���������� �����̵� ��ų ���ΰ�?)
	if (CurrentSpaceship)
	{
		// ���� �������� ���� �ٶ��� �ʰ��� ž���ϰ� �Ϸ��� �� ���� ����.
		// ������ ��Ƽ�÷��̾� ȯ�濡�� ���۵� ���ɼ��� �־� �������� �ʽ��ϴ�.
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

	// [�߰�] ���� ���̸� �̵� �Է��� �����ϰ� ��������
	if (bIsActivelyRepairing)
	{
		return;
	}

	CurrentInputVector = Value.Get<FVector2D>();
	// 2. [�߰�] �������׵� �˷���!
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
	// 2. [�߰�] �������� ����ٰ� �˷���!
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
	// ��Ŀ�� �����̰�, �̹� ���� ���� �ƴ� ���� �ߵ�
	if (ReplicatedRelativeData.bIsAnchored && !bIsJumping)
	{
		bIsJumping = true;
		CurrentVerticalSpeed = JumpInitialSpeed;
		// ���� ��� �ٴ� ���� ���� ó���� Mode ������ ���� ���� (User Request)
		// ���� bIsJumping �÷��׷θ� ����

		// ������ �α�
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

	// ���� ������ BaseActor�� ������ �˱� ������ �� ���ǹ��� ����˴ϴ�!
	// -> ���� ĳ���͵� ȸ���ϱ� ������.
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

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		TaskPawn->SetPilot(this);
		PC->Possess(TaskPawn);
		TaskPawn->Client_BoardingSuccess();
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
		// ���� ǥ�� �Լ��� �Է� �ý��� ��õ�
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

		// Ȥ�� �� ������ġ: ���� ��ǲ ���� ȣ��
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
		// 1. �ͷ��� ������ ���� ���
		TurretToBoard->SetPilot(this, LinkedCockpit);

		// 2. ��Ʈ�ѷ� ���� (Possess) - ���� ĳ���Ͱ� �ƴ� �ͷ��� ����
		PC->Possess(TurretToBoard);

		// 3. Ŭ���̾�Ʈ ȭ��/�Է� ��ȯ ����
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
	// 1. ������ ���� (���� ������ BaseActor�� ������ �˰� ��)
	ReplicatedRelativeData.BaseActor = NewBase;
	ReplicatedRelativeData.bIsAnchored = (NewBase != nullptr);

	if (NewBase)
	{
		// 2. ���� �������� ������ ���� ����
		AttachToActor(NewBase, FAttachmentTransformRules::KeepWorldTransform);

		// 3. [�ٽ�] ���� ���� ������ ���� ����
		// DisableMovement()�� ���� ������. ��� Custom ���� ��ȯ.
		GetCharacterMovement()->SetMovementMode(MOVE_Custom);

		// 4. [�ٽ�] "�������� ��ġ ����ȭ�� RPC�� �������� �� �״�, ���� �ʴ� ����"
		SetReplicateMovement(false);
	}
	else
	{
		// ���� ���� �� ����
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(true);
	}
}

// �Է� ó�� �Լ� (Ŭ���̾�Ʈ)
void APSJ_Character::Input_ForceEject(const FInputActionValue& Value)
{
	if (CurrentSpaceship) return;

	if (EquippedTool && EquippedTool->bIsOnCooldown)
	{
		Server_TryForceEject(nullptr);
		return;
	}

	// 1. Ʈ���̽� ������: ĳ���� �߽� ��ġ + ĳ���Ͱ� �ٶ󺸴� ���� ���� ������ ����
	FVector StartLoc = GetActorLocation() + GetActorRotation().RotateVector(ForceEjectSphereOffset);

	// 2. Ʈ���̽� ����: ���������� ����(Forward)���� Range��ŭ �̵�
	FVector EndLoc = StartLoc + (GetActorForwardVector() * ForceEjectRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	// 3. ���Ǿ� ����(CollisionShape) ����
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ForceEjectSphereRadius);

	// 4. LineTrace ��� SweepSingleByChannel ���
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
	// ����� �����: Ʈ���̽��� ������ ������ ĸ�� ������� �׷��� ���������� Ȯ�� �����ϰ� ��
	FVector TraceVec = EndLoc - StartLoc;
	float TraceLen = TraceVec.Size();
	FVector CenterLoc = StartLoc + TraceVec * 0.5f;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();

	// ��Ʈ �����ϸ� �ʷϻ�, ����̸� ������ ĸ��
	DrawDebugCapsule(GetWorld(), CenterLoc, TraceLen * 0.5f + ForceEjectSphereRadius, ForceEjectSphereRadius, CapsuleRot, bHit ? FColor::Green : FColor::Red, false, 2.0f);

	if (bHit)
	{
		// ��Ȯ�� ��� �¾Ҵ��� �ʷϻ� ������ ǥ��
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 2.0f);
	}
#endif

	// 1. ����� ���� �ӽ� ������ nullptr�� �ʱ�ȭ�մϴ�. (����� �� ���¸� �⺻������ ��)
	APSJ_ShipCockpit* HitCockpit = nullptr;

	// 2. ���𰡿� �¾Ұ�, �װ� �����̶�� ������ ����ݴϴ�.
	if (bHit && HitResult.GetActor())
	{
		HitCockpit = Cast<APSJ_ShipCockpit>(HitResult.GetActor());
	}

	// 3. [�ٽ�] if�� ������ �����ϴ�! 
	// Ÿ���� ã�ҵ�(HitCockpit), �� ã�ҵ�(nullptr) ������ ������ �����Ͽ� Ÿ�̸Ӹ� �����ϴ�.
	Server_TryForceEject(HitCockpit);
}

// 3. ���� RPC ����
bool APSJ_Character::Server_TryForceEject_Validate(APSJ_ShipCockpit* TargetCockpit)
{
	if (TargetCockpit)
	{
		float DistanceSq = FVector::DistSquared(GetActorLocation(), TargetCockpit->GetActorLocation());

		// ��� �Ÿ� = �⺻ Range + ���Ǿ� ������ + ������ ���� + ���� ���� ���� ������(200.0f)
		float MaxAllowedDistance = ForceEjectRange + ForceEjectSphereRadius + ForceEjectSphereOffset.Size() + 200.0f;
		float AllowedRangeSq = FMath::Square(MaxAllowedDistance);

		if (DistanceSq > AllowedRangeSq)
		{
			return false; // �� ���̳� ���������� ��ġ������ ��û ����
		}
	}
	return true;
}

void APSJ_Character::Server_TryForceEject_Implementation(APSJ_ShipCockpit* TargetCockpit)
{
	if (EquippedTool)
	{
		if (EquippedTool->bIsOnCooldown)
		{
			// [����] ��ٿ� ��: ���� ����Ʈ ��� (���� ���� �� ��)
			EquippedTool->Multicast_PlayEjectEffect(false, this);
			return;
		}
		else
		{
			// [����] ���� ����: ����Ʈ ��� �� Ÿ�̸� ����
			EquippedTool->Multicast_PlayEjectEffect(true, this);
			EquippedTool->StartCooldownTimer();

			// ���� ���� ���� ���� ����
			if (TargetCockpit) {
				TargetCockpit->ReceiveForceEjectRequest();
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

	// ��Ʈ�ѷ� ��������
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 1. ĳ���� ������ ���߱� (���û���)
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->StopMovementImmediately();
			GetCharacterMovement()->DisableMovement();
		}

		// 2. �г�(TargetPawn) ���� ����!
		PC->Possess(TargetPawn);

		// (����) ���� �г� �ʿ��� "ž�� �Ϸ�Ǿ����ϴ�" ���� ó���� �ʿ��ϸ�
		// ���⼭ TargetPawn->OnBoarded() ���� �Լ��� ȣ������ ���� �ֽ��ϴ�.
	}
}

void APSJ_Character::Client_RestoreInputRPC_Implementation()
{
	// �� �ڵ�� Ŭ���̾�Ʈ ��ǻ�Ϳ��� ����˴ϴ�.
	// ������ ������ "�Է� ���� ���� �Լ�"�� �����Ͽ� ���콺/Ű���带 Ȱ��ȭ�մϴ�.
	ForceInputRecovery();

	// Ȥ�� �� ������ġ: ���� ǥ�� �Է� ����� �Լ��� ���� ȣ��
	PawnClientRestart();

	// �α׷� Ȯ��
	// UE_LOG(LogTemp, Warning, TEXT("[RPC] Client Input Restored via Blueprint Request!"));
}

// =========================================================
// [�ű�] ���� ���� ������
// =========================================================

void APSJ_Character::Input_StartRepair(const FInputActionValue& Value)
{
	bIsRepairingInputDown = true;
}

void APSJ_Character::Input_StopRepair(const FInputActionValue& Value)
{
	bIsRepairingInputDown = false;
	bIsActivelyRepairing = false; // ���� ���� �� false

	if (ClientRepairTarget)
	{
		Server_StopRepair();
		ClientRepairTarget = nullptr;
	}
}

void APSJ_Character::UpdateRepairLogic()
{
	// 1. ��ư�� �� ������ ������ �ƹ��͵� �� ��
	if (!bIsRepairingInputDown) return;

	// 2. �ü� Ʈ���̽� (RepairTraceLength ���)
	FVector TraceStart;
	FRotator TraceRot;

	// [����] ���� Ʈ���̽��� ������ ĳ���� ����(ī�޶�)���� �߻�
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

	// ���ü�(Visibility) ä�η� üũ
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// =========================================================
	// [�ű�] ���־� ����� ���� �׸��� (��Ŭ�� ���� �� ����)
	// =========================================================
	if (bHit)
	{
		// �浹 �������� �ʷϻ� ��
		DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Green, false, 0.1f, 0, 1.0f);
		// �浹 ��ġ�� �� ���
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 0.1f);
	}
	else
	{
		// ����� ������ ��
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
	}
	// =========================================================

	APSJ_ShipCockpit* HitCockpit = nullptr;
	if (bHit && HitResult.GetActor())
	{
		HitCockpit = Cast<APSJ_ShipCockpit>(HitResult.GetActor());
	}

	// 3. ���� ���� ���� �Ǵ�
	bool bCanRepair = false;

	if (HitCockpit)
	{
		// (1) ���峭 �����ΰ�?
		if (HitCockpit->bIsMalfunctioning)
		{
			// (2) �Ÿ��� ����? (RepairMaxDistance üũ)
			float Dist = FVector::Dist(GetActorLocation(), HitCockpit->GetActorLocation());
			if (Dist <= RepairMaxDistance)
			{
				bCanRepair = true;
			}
			else
			{
				// [���� ����] ȭ�鿡 "�ʹ� ����!" �޽��� ���� ����
				// PrintString: Too Far to Repair!
			}
		}
	}

	// 4. ���� ��ȭ ó��
	if (bCanRepair)
	{
		// [�߰�] ��� �� ������ ������ ���̶�� �̵� ���� ����
		if (!bIsActivelyRepairing)
		{
			// �Է� ���� �ʱ�ȭ �� ���� ����
			CurrentInputVector = FVector2D::ZeroVector;
			Server_SetInputVector(FVector2D::ZeroVector);

			// �������� �̵� ���ӵ�(����) ��� ����
			GetCharacterMovement()->Velocity = FVector::ZeroVector;
		}

		bIsActivelyRepairing = true; // ���� ���� Ȱ��ȭ

		if (ClientRepairTarget != HitCockpit)
		{
			if (ClientRepairTarget) Server_StopRepair();
			Server_StartRepair(HitCockpit);
			ClientRepairTarget = HitCockpit;
		}
	}
	else
	{
		bIsActivelyRepairing = false;

		if (ClientRepairTarget != nullptr)
		{
			Server_StopRepair();
			ClientRepairTarget = nullptr;
		}
	}
}

// [����] ���� ����
bool APSJ_Character::Server_StartRepair_Validate(APSJ_ShipCockpit* TargetCockpit) { return true; }
void APSJ_Character::Server_StartRepair_Implementation(APSJ_ShipCockpit* TargetCockpit)
{
	if (TargetCockpit)
	{
		// ���Ϳ� ���� ��� (���� �ο� +1)
		TargetCockpit->AddRepairer(this);

		// ������ ���� ���� �����ϴ��� ����ص� (���߿� ���� �� ���)
		ServerRepairTarget = TargetCockpit;
	}
	// [�߰�] ������ "�� ���� ����"�̶�� ���� �� ��ο��� ����
	bIsActivelyRepairing = true;
}

// [����] ���� �ߴ�
bool APSJ_Character::Server_StopRepair_Validate() { return true; }
void APSJ_Character::Server_StopRepair_Implementation()
{
	// ���� ����ϰ� �ִ� ���Ϳ��Լ� ���� ����
	if (ServerRepairTarget)
	{
		ServerRepairTarget->RemoveRepairer(this);
		ServerRepairTarget = nullptr;
	}
	// [�߰�] ���� �����ٰ� ��ο��� ����
	bIsActivelyRepairing = false;
}

// 2. ���� �� �Ʒ�(Ȥ�� ���� ��)�� �����θ� �߰��ϼ���.
void APSJ_Character::Input_SprintStart(const FInputActionValue& Value)
{
	bIsSprinting = true;
}

void APSJ_Character::Input_SprintStop(const FInputActionValue& Value)
{
	bIsSprinting = false;
}

// --- �Է� ���� ����ȭ RPC ---
bool APSJ_Character::Server_SetInputVector_Validate(FVector2D NewInput)
{
	return true;
}

void APSJ_Character::Server_SetInputVector_Implementation(FVector2D NewInput)
{
	// ������ Ŭ���̾�Ʈ�� �Է°��� �޾Ƽ� �ڱ� ������ ������Ʈ
	CurrentInputVector = NewInput;
}

// --- �޸��� ����ȭ RPC ---
bool APSJ_Character::Server_SetSprinting_Validate(bool bNewSprinting)
{
	return true;
}

void APSJ_Character::Server_SetSprinting_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
}