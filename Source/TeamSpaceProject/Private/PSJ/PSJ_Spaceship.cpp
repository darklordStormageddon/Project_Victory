#include "PSJ_Spaceship.h"
#include "PSJ_Character.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

APSJ_Spaceship::APSJ_Spaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	// [수정 안 함] BP 컴포넌트를 사용할 것이므로 C++에서 생성하지 않고 포인터만 초기화
	ShipRootComponent = nullptr;
	PilotCamera = nullptr;
	PilotSphere = nullptr;
	ExitPoint = nullptr;
}

void APSJ_Spaceship::BeginPlay()
{
	Super::BeginPlay();

	// =============================================================
	// BP 컴포넌트 연결 (이름이나 타입으로 찾기)
	// =============================================================

	// 1. RootComponent 연결
	if (RootComponent)
	{
		ShipRootComponent = Cast<UPrimitiveComponent>(RootComponent);
	}

	// 2. 카메라 연결
	PilotCamera = FindComponentByClass<UCameraComponent>();
	if (!PilotCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Camera not found in BP"));
	}

	// 3. [핵심] ExitPoint (Arrow) 연결
	// BP에 있는 ArrowComponent 중 "Exit"가 이름에 들어가거나, 혹은 첫 번째 Arrow를 가져옵니다.
	TArray<UArrowComponent*> Arrows;
	GetComponents(Arrows);
	for (UArrowComponent* Arrow : Arrows)
	{
		// BP 컴포넌트 이름이 ExitPoint라면 그걸 우선적으로 사용
		if (Arrow->GetName().Contains(TEXT("Exit")) || Arrows.Num() == 1)
		{
			ExitPoint = Arrow;
			break;
		}
	}
	if (!ExitPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("Error: ExitPoint (Arrow) not found in BP!"));
	}

	// 4. [핵심] Sphere (PilotSphere) 연결 및 이벤트 바인딩
	// 이게 연결되어야 탑승이 가능합니다.
	PilotSphere = FindComponentByClass<USphereComponent>();
	if (PilotSphere)
	{
		// 오버랩 이벤트 동적 바인딩 (이 부분이 빠져서 탑승이 안됐던 것 복구)
		PilotSphere->OnComponentBeginOverlap.AddDynamic(this, &APSJ_Spaceship::OnOverlapBegin);
		PilotSphere->OnComponentEndOverlap.AddDynamic(this, &APSJ_Spaceship::OnOverlapEnd);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Sphere Component not found in BP"));
	}
}

void APSJ_Spaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APSJ_Spaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 기존 비행 조작
		if (IA_ThrustForward) EnhancedInputComponent->BindAction(IA_ThrustForward, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_ThrustForward);
		if (IA_ThrustBackward) EnhancedInputComponent->BindAction(IA_ThrustBackward, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_ThrustBackward);
		if (IA_MoveAxes) EnhancedInputComponent->BindAction(IA_MoveAxes, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_MoveAxes);
		if (IA_MoveUp) EnhancedInputComponent->BindAction(IA_MoveUp, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_MoveUp);
		if (IA_MouseLook) EnhancedInputComponent->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_MouseLook);
		if (IA_Roll) EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_Roll);

		// 하차 키
		if (IA_Interact) EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Started, this, &APSJ_Spaceship::Input_Exit);
	}
}

void APSJ_Spaceship::SetPilot(APSJ_Character* NewPilot)
{
	CurrentPilot = NewPilot;
	if (CurrentPilot)
	{
		if (APlayerController* PC = Cast<APlayerController>(CurrentPilot->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->ClearAllMappings();
				if (ShipMappingContext)
				{
					Subsystem->AddMappingContext(ShipMappingContext, 0);
				}
			}
		}
	}
}

// =============================================================
// [복구] 탑승 감지 로직 (SetCurrentSpaceship 호출)
// =============================================================
void APSJ_Spaceship::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 자기 자신이 아니고, 캐릭터일 경우
	if (OtherActor && OtherActor != this)
	{
		if (APSJ_Character* Character = Cast<APSJ_Character>(OtherActor))
		{
			// 캐릭터에게 "나(이 우주선)를 현재 대상"으로 등록
			Character->SetCurrentSpaceship(this);
		}
	}
}

void APSJ_Spaceship::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor != this)
	{
		if (APSJ_Character* Character = Cast<APSJ_Character>(OtherActor))
		{
			// 이미 탑승한 상태가 아니라면 등록 해제
			if (CurrentPilot != Character)
			{
				Character->SetCurrentSpaceship(nullptr);
			}
		}
	}
}

// =============================================================
// [수정] 하차 로직 (ExitPoint 사용 + 물리 초기화)
// =============================================================
void APSJ_Spaceship::Input_Exit(const FInputActionValue& Value)
{
	DisembarkCharacter();
}

void APSJ_Spaceship::DisembarkCharacter()
{
	if (!CurrentPilot) return;

	// ExitPoint가 없으면 경고 띄우고 현재 위치에서 내리거나 리턴 (안전을 위해 체크)
	if (!ExitPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("ExitPoint is NULL! Character will verify default location."));
	}

	APSJ_Character* ExitingChar = CurrentPilot;

	// 1. 컨트롤러 반환
	if (AController* ShipController = GetController())
	{
		ShipController->Possess(ExitingChar);
	}

	// 2. 관계 해제
	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 3. 위치 이동 (ExitPoint가 있으면 거기로, 없으면 현재 위치 유지)
	if (ExitPoint)
	{
		FVector SpawnLoc = ExitPoint->GetComponentLocation();
		FRotator SpawnRot = ExitPoint->GetComponentRotation();

		// 충돌 무시 (튕김 방지)
		this->MoveIgnoreActorAdd(ExitingChar);
		ExitingChar->MoveIgnoreActorAdd(this);

		// 텔레포트 (물리 왜곡 방지)
		ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);
	}

	// 4. 물리 속도 초기화 (날아감 방지)
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(ExitingChar->GetRootComponent()))
	{
		RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
		RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
	if (ExitingChar->GetCharacterMovement())
	{
		ExitingChar->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		//ExitingChar->GetCharacterMovement()->StopMovementImmediately();
		ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Flying); // 혹은 Walking
	}

	// 5. 캐릭터 활성화
	ExitingChar->SetActorHiddenInGame(false);
	ExitingChar->SetActorEnableCollision(true);

	// 6. 충돌 복구 타이머
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &APSJ_Spaceship::EnableCollisionWithPassenger, ExitingChar);
	GetWorld()->GetTimerManager().SetTimer(CollisionResetTimerHandle, TimerDel, 2.0f, false);

	CurrentPilot = nullptr;
}

void APSJ_Spaceship::EnableCollisionWithPassenger(APSJ_Character* ExitedChar)
{
	if (ExitedChar && IsValid(ExitedChar))
	{
		this->MoveIgnoreActorRemove(ExitedChar);
		ExitedChar->MoveIgnoreActorRemove(this);
	}
}

// === 이동 로직 (기존 유지) ===
void APSJ_Spaceship::Input_ThrustForward(const FInputActionValue& Value)
{
	if (ShipRootComponent) ShipRootComponent->AddForce(GetActorForwardVector() * ThrustSpeed * Value.Get<float>(), NAME_None, true);
}
void APSJ_Spaceship::Input_ThrustBackward(const FInputActionValue& Value)
{
	if (ShipRootComponent) ShipRootComponent->AddForce(GetActorForwardVector() * -ThrustSpeed * Value.Get<float>(), NAME_None, true);
}
void APSJ_Spaceship::Input_MoveAxes(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (ShipRootComponent)
	{
		FVector RightForce = GetActorRightVector() * MovementVector.X * ThrustSpeed * 0.5f;
		FVector UpForce = GetActorUpVector() * MovementVector.Y * ThrustSpeed * 0.5f;
		ShipRootComponent->AddForce(RightForce + UpForce, NAME_None, true);
	}
}
void APSJ_Spaceship::Input_MoveUp(const FInputActionValue& Value)
{
	// 1D Axis 값을 가져옴 (E키: 1.0, Q키: -1.0 으로 설정한다고 가정)
	float UpValue = Value.Get<float>();

	if (ShipRootComponent)
	{
		// GetActorUpVector(): 우주선의 '지붕' 방향 벡터
		// UpValue가 양수면 위로, 음수면 아래로 힘을 가함
		// 기존 Input_MoveAxes와 균형을 맞추기 위해 ThrustSpeed * 0.5f를 동일하게 적용
		FVector UpForce = GetActorUpVector() * UpValue * ThrustSpeed * 0.5f;

		ShipRootComponent->AddForce(UpForce, NAME_None, true);
	}
}
void APSJ_Spaceship::Input_MouseLook(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	AddActorLocalRotation(FRotator(LookVector.Y * -1.0f, LookVector.X, 0.0f));
}
void APSJ_Spaceship::Input_Roll(const FInputActionValue& Value)
{
	AddActorLocalRotation(FRotator(0.0f, 0.0f, Value.Get<float>() * RotateSpeed));
}