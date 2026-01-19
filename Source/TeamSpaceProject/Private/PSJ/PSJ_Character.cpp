// Fill out your copyright notice in the Description page of Project Settings.


#include "PSJ_Character.h"
#include "PSJ_Spaceship.h" // 우주선 헤더
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"


APSJ_Character::APSJ_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();

	// 1. 컨트롤러 회전 강제 비활성화
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// 2. Flying 모드 설정
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->BrakingDecelerationFlying = 10000.0f;
	GetCharacterMovement()->MaxFlySpeed = 600.0f;

	GetCharacterMovement()->bImpartBaseVelocityX = false;
	GetCharacterMovement()->bImpartBaseVelocityY = false;
	GetCharacterMovement()->bImpartBaseVelocityZ = false;
	GetCharacterMovement()->bImpartBaseAngularVelocity = false;

	// 3. 카메라 컴포넌트 찾기
	FPSCamera = FindComponentByClass<UCameraComponent>();
	if (!FPSCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: No Camera Component found on Character BP!"));
	}

	if (GetMesh())
	{
		DefaultMeshZ = GetMesh()->GetRelativeLocation().Z;
	}

}

void APSJ_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateMagBoots(DeltaTime);
}

void APSJ_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// IMC(키 매핑)를 여기서 등록해야 하차 후에도 키가 먹gla
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// 기존 매핑 비우고 다시 등록 (우주선 키랑 꼬이지 않게)
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
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APSJ_Character::Move);

		if (LookAction)
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APSJ_Character::Look);

		if (InteractAction)
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &APSJ_Character::Interact);
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
				// 1. 우주선에 조종사 등록
				TargetShip->SetPilot(this);

				// 2. 물리 충돌 끄기
				SetActorEnableCollision(false);

				// 3. 캐릭터 숨기기
				// SetActorHiddenInGame(true); 

				// 4. 우주선에 부착
				// AttachToActor(TargetShip, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
				AttachToActor(TargetShip, FAttachmentTransformRules::KeepWorldTransform);

				// 5. 빙의
				PC->Possess(TargetShip);

				UE_LOG(LogTemp, Warning, TEXT("=== SUCCESS: Boarded Spaceship ==="));
			}
		}
	}
}

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{
	float TraceRadius = 25.0f;
	// [확인] 400.0f면 충분히 깁니다. (기존 200 -> 400 유지)
	float CheckLen = 400.0f;

	FHitResult FinalHit;
	bool bFoundValidHit = false;
	FVector Start = GetActorLocation();
	FVector End = Start + (-GetActorUpVector() * CheckLen);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(TraceRadius);
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
	bIsMagBootsActive = bFoundValidHit;
	LastFloorActor = bFoundValidHit ? FinalHit.GetActor() : nullptr;

	if (bFoundValidHit)
	{
		UCharacterMovementComponent* CMC = GetCharacterMovement();
		UPrimitiveComponent* FloorComp = FinalHit.GetComponent();

		// [핵심] 우주선 이동 따라가기 (Moving Platform)
		if (FloorComp && CMC->GetMovementBase() != FloorComp)
		{
			CMC->SetBase(FloorComp, FinalHit.BoneName);
		}

		CurrentFloorNormal = FinalHit.ImpactNormal;
		float HoverOffset = 0.0f;
		FVector TargetUp = FinalHit.ImpactNormal;
		bool bIsStairs = false;

		if (FinalHit.Component.IsValid() && FinalHit.Component->ComponentTags.Contains("Stairs"))
		{
			bIsStairs = true;
			HoverOffset = 15.0f;
			if (AActor* Ship = FinalHit.GetActor())
			{
				TargetUp = Ship->GetActorUpVector();
			}
		}
		FVector CurrentUp = GetActorUpVector();
		FQuat CurrentRot = GetActorQuat();
		FQuat DeltaRot = FQuat::FindBetweenNormals(CurrentUp, TargetUp);
		FQuat TargetRot = DeltaRot * CurrentRot;
		FQuat NewRot = FMath::QInterpTo(CurrentRot, TargetRot, DeltaTime, AlignSpeed);
		SetActorRotation(NewRot);

		float TargetHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		if (bIsStairs)
		{
			// Stairs일 때 캡슐을 띄워서 충돌 회피
			TargetHeight += 15.0f;
		}
		float ActualDistanceToFloor = FinalHit.Distance + TraceRadius;
		float Error = TargetHeight - ActualDistanceToFloor;
		float VerticalVelocity = FVector::DotProduct(GetVelocity(), TargetUp);
		float SpringForce = Error * SpringStiffness;
		float DampingForce = VerticalVelocity * SpringDamping;
		FVector SuspensionAccel = TargetUp * (SpringForce - DampingForce);
		GetCharacterMovement()->Velocity += SuspensionAccel * DeltaTime;

		// [시각 보정] 물리적으로 뜬 만큼 메쉬를 내림
		if (GetMesh())
		{
			FVector CurrentRelLoc = GetMesh()->GetRelativeLocation();
			float TargetMeshZ = DefaultMeshZ - HoverOffset;
			float NewMeshZ = FMath::FInterpTo(CurrentRelLoc.Z, TargetMeshZ, DeltaTime, 15.0f);
			GetMesh()->SetRelativeLocation(FVector(CurrentRelLoc.X, CurrentRelLoc.Y, NewMeshZ));
		}
	}
	else
	{
		// [수정 2] 바닥을 놓쳤을 때 물리적 연결을 확실히 끊어야 함!
		// 이게 없으면 사거리(400) 밖으로 나가도 우주선 회전에 계속 끌려다니거나 공중에 멈춤
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetBase(nullptr);
		}

		// 바닥이 없을 때 메쉬 복구
		if (GetMesh())
		{
			FVector CurrentRelLoc = GetMesh()->GetRelativeLocation();
			float NewMeshZ = FMath::FInterpTo(CurrentRelLoc.Z, DefaultMeshZ, DeltaTime, 10.0f);
			GetMesh()->SetRelativeLocation(FVector(CurrentRelLoc.X, CurrentRelLoc.Y, NewMeshZ));
		}

		bIsMagBootsActive = false;
		LastFloorActor = nullptr;
	}
}

void APSJ_Character::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr && FPSCamera != nullptr)
	{
		FVector UpVector = GetActorUpVector();
		FVector CameraForward = FPSCamera->GetForwardVector();
		FVector RightVector = FVector::CrossProduct(UpVector, CameraForward);
		if (RightVector.IsNearlyZero())
		{
			RightVector = FVector::CrossProduct(UpVector, GetActorForwardVector());
		}
		RightVector.Normalize();
		FVector ForwardVector = FVector::CrossProduct(RightVector, UpVector);
		ForwardVector.Normalize();

		// [수정 핵심] 자석 신발이 활성화된 상태라면, 이동 벡터를 바닥 경사에 맞춘다.
		// ==========================================================
		if (bIsMagBootsActive)
		{
			// ForwardVector를 바닥 기울기 평면에 투영
			ForwardVector = FVector::VectorPlaneProject(ForwardVector, CurrentFloorNormal);
			ForwardVector.Normalize();

			// RightVector도 바닥 기울기 평면에 투영 (옆걸음질 시 경사 타기 위해)
			RightVector = FVector::VectorPlaneProject(RightVector, CurrentFloorNormal);
			RightVector.Normalize();
		}

		FVector StartLine = GetActorLocation();
		DrawDebugDirectionalArrow(GetWorld(), StartLine, StartLine + (CameraForward * 150.0f), 50.0f, FColor::Blue, false, -1.0f, 0, 5.0f);
		DrawDebugDirectionalArrow(GetWorld(), StartLine, StartLine + (ForwardVector * 150.0f), 50.0f, FColor::Green, false, -1.0f, 0, 5.0f);
		DrawDebugDirectionalArrow(GetWorld(), StartLine, StartLine + (UpVector * 100.0f), 30.0f, FColor::Yellow, false, -1.0f, 0, 3.0f);

		AddMovementInput(ForwardVector, MovementVector.Y);
		AddMovementInput(RightVector, MovementVector.X);
	}
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