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
		if (UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
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

	if (IsLocallyControlled())
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

		if (!bJustDisembarked)
		{
			if (!HasAuthority())
				Server_UpdateRelativeTransform(GetRootComponent()->GetRelativeLocation(), GetRootComponent()->GetRelativeRotation());
			else
			{
				ReplicatedRelativeData.RelativeLocation = GetRootComponent()->GetRelativeLocation();
				ReplicatedRelativeData.RelativeRotation = GetRootComponent()->GetRelativeRotation();
			}
		}
	}
	else
	{
		if (ReplicatedRelativeData.BaseActor)
		{
			FVector OldRelLocation = GetRootComponent()->GetRelativeLocation();

			SetActorRelativeLocation(ReplicatedRelativeData.RelativeLocation);
			SetActorRelativeRotation(ReplicatedRelativeData.RelativeRotation);

			if (DeltaTime > KINDA_SMALL_NUMBER)
			{
				FVector RelDelta = (FVector(ReplicatedRelativeData.RelativeLocation) - OldRelLocation) / DeltaTime;
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

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	if (!IsLocallyControlled()) return;

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



void APSJ_Character::Move(const FInputActionValue& Value)
{

	if (bIsActivelyRepairing)
	{
		return;
	}

	CurrentInputVector = Value.Get<FVector2D>();
	Server_SetInputVector(CurrentInputVector);

	if (!CurrentInputVector.IsNearlyZero())
	{
		FString ModeString = UEnum::GetValueAsString(GetCharacterMovement()->MovementMode);
		FVector Vel = GetVelocity();
	}
}

void APSJ_Character::StopMove(const FInputActionValue& Value)
{
	CurrentInputVector = FVector2D::ZeroVector;
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

		SetReplicateMovement(false);
	}
	else
	{

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		SetReplicateMovement(true);
	}
}

void APSJ_Character::Input_ForceEject(const FInputActionValue& Value)
{
	if (CurrentSpaceship) return;

	if (EquippedTool && EquippedTool->bIsOnCooldown)
	{
		Server_TryForceEject(nullptr);
		return;
	}


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


	Server_TryForceEject(TargetChair);
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
	if (!bIsRepairingInputDown) return;

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

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bHit)
	{

		DrawDebugLine(GetWorld(), TraceStart, HitResult.ImpactPoint, FColor::Green, false, 0.1f, 0, 1.0f);

		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Green, false, 0.1f);
	}
	else
	{

		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red, false, 0.1f, 0, 1.0f);
	}
	// =========================================================

	ATaskChair* TargetChair = nullptr;
	if (bHit && HitResult.GetActor())
	{
		TargetChair = Cast<ATaskChair>(HitResult.GetActor());
	}


	bool bCanRepair = false;

	if (TargetChair)
	{

		if (TargetChair->bIsMalfunctioning)
		{

			float Dist = FVector::Dist(GetActorLocation(), TargetChair->GetActorLocation());
			if (Dist <= RepairMaxDistance)
			{
				bCanRepair = true;
			}
			else
			{

			}
		}
	}


	if (bCanRepair)
	{

		if (!bIsActivelyRepairing)
		{

			CurrentInputVector = FVector2D::ZeroVector;
			Server_SetInputVector(FVector2D::ZeroVector);


			GetCharacterMovement()->Velocity = FVector::ZeroVector;
		}

		bIsActivelyRepairing = true; 

		if (ClientRepairTarget != TargetChair)
		{
			if (ClientRepairTarget) Server_StopRepair();
			Server_StartRepair(TargetChair);
			ClientRepairTarget = TargetChair;
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