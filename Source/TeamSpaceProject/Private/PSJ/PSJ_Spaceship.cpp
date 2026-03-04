#include "PSJ_Spaceship.h"
#include "PSJ/TaskChair.h"
#include "PSJ_Character.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "KSM/HealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "CJH/Component/DistanceComponent.h"
#include "JHS/SpaceObject/DriveSeatRader.h"
#include "Kismet/GameplayStatics.h"
#include "JHS/Event/EventManager.h"

APSJ_Spaceship::APSJ_Spaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = true;
	SetReplicateMovement(true);

	NetUpdateFrequency = 60.0f;
	MinNetUpdateFrequency = 30.0f;
	//bReplicatePhysicsToAutonomousProxy = false;


	ShipRootComponent = nullptr;
	PilotCamera = nullptr;
	PilotSphere = nullptr;


	DistanceComp = CreateDefaultSubobject<UDistanceComponent>(TEXT("DistanceComponent"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APSJ_Spaceship::Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot)
{
	Super::Client_BoardingSuccess_Implementation(BoardingPilot); // 부모 로직 실행

	// 우주선 전용 조작키 설정
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
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


void APSJ_Spaceship::Input_ThrustForward(const FInputActionValue& Value)
{
	float Val = Value.Get<float>();

	// 1. 내가 조종 중인 클라이언트라면 즉시 물리 힘 적용 (예측)
	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		ShipRootComponent->AddForce(GetActorForwardVector() * ThrustSpeed * Val, NAME_None, true);
	}

	// 2. 서버에도 동일하게 적용하라고 명령
	Server_ThrustForward(Val);
}

bool APSJ_Spaceship::Server_ThrustForward_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_ThrustForward_Implementation(float Value)
{
	if (!TryMove()) return;
		ShipRootComponent->AddForce(GetActorForwardVector() * ThrustSpeed * Value, NAME_None, true);
}

void APSJ_Spaceship::Input_ThrustBackward(const FInputActionValue& Value)
{
	Server_ThrustBackward(Value.Get<float>());
}

bool APSJ_Spaceship::Server_ThrustBackward_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_ThrustBackward_Implementation(float Value)
{
	if (!TryMove()) return;
		ShipRootComponent->AddForce(GetActorForwardVector() * -ThrustSpeed * Value, NAME_None, true);
}

void APSJ_Spaceship::Input_MoveAxes(const FInputActionValue& Value)
{
	Server_MoveAxes(Value.Get<FVector2D>());
}

bool APSJ_Spaceship::Server_MoveAxes_Validate(FVector2D Value) { return true; }

void APSJ_Spaceship::Server_MoveAxes_Implementation(FVector2D Value)
{
	if (!TryMove()) return;

	FVector RightForce = GetActorRightVector() * Value.X * ThrustSpeed * 0.5f;
	FVector UpForce = GetActorUpVector() * Value.Y * ThrustSpeed * 0.5f;
	ShipRootComponent->AddForce(RightForce + UpForce, NAME_None, true);
}

void APSJ_Spaceship::Input_MoveUp(const FInputActionValue& Value)
{
	Server_MoveUp(Value.Get<float>());
}

bool APSJ_Spaceship::Server_MoveUp_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_MoveUp_Implementation(float Value)
{
	if (!TryMove()) return;
	FVector UpForce = GetActorUpVector() * Value * ThrustSpeed * 0.5f;
	ShipRootComponent->AddForce(UpForce, NAME_None, true);
}

void APSJ_Spaceship::Input_Roll(const FInputActionValue& Value)
{
	float Val = Value.Get<float>();

	// 1. 클라이언트 로컬 예측: 즉시 물리적 회전력(Torque) 적용
	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		// Roll은 앞/뒤 축(Forward Vector)을 기준으로 회전
		ShipRootComponent->AddTorqueInDegrees(GetActorForwardVector() * Val * RotateSpeed, NAME_None, true);
	}

	// 2. 서버 통지
	Server_Roll(Val);
}

bool APSJ_Spaceship::Server_Roll_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_Roll_Implementation(float Value)
{
	if (!TryMove() || !ShipRootComponent) return;

	// 서버에서도 동일한 회전력 적용
	ShipRootComponent->AddTorqueInDegrees(GetActorForwardVector() * Value * RotateSpeed, NAME_None, true);
}

void APSJ_Spaceship::Input_MouseLook(const FInputActionValue& Value)
{
	FVector2D Val = Value.Get<FVector2D>();

	// [개선] 기존에 누락되었던 클라이언트 측 즉각 반응(예측) 추가
	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		// Pitch(위아래)는 우측 축(Right Vector) 기준, Yaw(좌우)는 위쪽 축(Up Vector) 기준
		FVector PitchTorque = GetActorRightVector() * (Val.Y * -1.0f) * RotateSpeed;
		FVector YawTorque = GetActorUpVector() * Val.X * RotateSpeed;

		ShipRootComponent->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, true);
	}

	// 서버 통지
	Server_MouseLook(Val);
}

bool APSJ_Spaceship::Server_MouseLook_Validate(FVector2D Value) { return true; }

void APSJ_Spaceship::Server_MouseLook_Implementation(FVector2D Value)
{
	if (!TryMove() || !ShipRootComponent) return;

	FVector PitchTorque = GetActorRightVector() * (Value.Y * -1.0f) * RotateSpeed;
	FVector YawTorque = GetActorUpVector() * Value.X * RotateSpeed;

	ShipRootComponent->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, true);
}

void APSJ_Spaceship::BeginPlay()
{
	Super::BeginPlay();

	// 우주선이 스폰된 최초의 위치와 회전(Transform)을 기억해 둡니다.
	InitialTransform = GetActorTransform();

	RegistEvent();

	if (HealthComp)
	{
		HealthComp->OnDamaged.AddDynamic(this, &APSJ_Spaceship::OnTakeDamage);
	}

	if (_outGameState == nullptr)
		UStaticFunctionLibrary::TryGetGameState(_outGameState);

	ShipRootComponent = Cast<UPrimitiveComponent>(RootComponent);

	if (ShipRootComponent)
	{
		ShipRootComponent->bReplicatePhysicsToAutonomousProxy = false;
	}

	TArray<UStaticMeshComponent*> StaticMeshes;
	GetComponents<UStaticMeshComponent>(StaticMeshes);

	for (UStaticMeshComponent* Mesh : StaticMeshes)
	{
		// 이름에 "Shield"가 포함된 메쉬를 찾음 (대소문자 무관)
		if (Mesh->GetName().Contains(TEXT("Shield")))
		{
			ShieldMesh = Mesh;
			// 쉴드 초기 상태 설정 (투명, 충돌 끄기)
			ShieldMesh->SetHiddenInGame(true);
			ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			UE_LOG(LogTemp, Log, TEXT("Spaceship: Found Shield Mesh successfully!"));
			break;
		}
	}


	PilotCamera = FindComponentByClass<UCameraComponent>();
	if (!PilotCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Camera not found in BP"));
	}


	TArray<UChildActorComponent*> ChildActors;
	GetComponents(ChildActors);

	for (UChildActorComponent* ChildComp : ChildActors)
	{
		if (ChildComp && ChildComp->GetChildActor())
		{
			if (ATaskChair* FoundChair = Cast<ATaskChair>(ChildComp->GetChildActor()))
			{

				LinkedChair = FoundChair;
			}
		}
	}

	PilotSphere = FindComponentByClass<USphereComponent>();
	if (PilotSphere)
	{
		PilotSphere->OnComponentBeginOverlap.AddDynamic(this, &APSJ_Spaceship::OnOverlapBegin);
		PilotSphere->OnComponentEndOverlap.AddDynamic(this, &APSJ_Spaceship::OnOverlapEnd);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Sphere Component not found in BP"));
	}


	if (DistanceComp)
		DistanceComp->OnDistanceDamaged.AddDynamic(this, &APSJ_Spaceship::OverDistanceDamageCheck);
	else
		UE_LOG(LogTemp, Warning, TEXT("Warning: Distance Component not found in BP"));


}

void APSJ_Spaceship::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregistEvent();

	Super::EndPlay(EndPlayReason);
}

bool APSJ_Spaceship::TryMove()
{
	USpaceShipStateGroup* StateGroup = GetSpaceShipStateGroup();
	if (StateGroup != nullptr)
	{
		return StateGroup->TryConsumeFuel();
	}
	return false; 
}

void APSJ_Spaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ShipRootComponent && ShipRootComponent->IsSimulatingPhysics())
	{
		FVector CurrentVelocity = ShipRootComponent->GetComponentVelocity();
		float CurrentSpeed = CurrentVelocity.Size();

		if (CurrentSpeed > MaxSpeed)
		{
			FVector ClampedVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;

			ShipRootComponent->SetPhysicsLinearVelocity(ClampedVelocity);
		}
	}

	if (GEngine)
	{
		FVector Velocity = GetVelocity();
		float SpeedCmPerSec = Velocity.Size();
		float SpeedKmh = SpeedCmPerSec * 0.036f;

		FString SpeedMsg = FString::Printf(TEXT("[Speed] %.2f cm/s  ( %.0f km/h )"), SpeedCmPerSec, SpeedKmh);
		GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, SpeedMsg);

		FString LocMsg = FString::Printf(TEXT("[Location] %s"), *GetActorLocation().ToString());
		GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Cyan, LocMsg);

		if (LinkedChair)
		{
			float DistanceToChair = FVector::Dist(GetActorLocation(), LinkedChair->GetActorLocation());
			FString GapMsg = FString::Printf(TEXT("[Chair Gap] %.2f (Is Lagging?)"), DistanceToChair);
			FColor GapColor = (SpeedCmPerSec > 10.0f) ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(3, 0.0f, GapColor, GapMsg);
		}
	}
}

void APSJ_Spaceship::OnTakeDamage(float Damage)
{
	ShowShield();

	if (GetSpaceShipStateGroup())
		_spaceShipStateGroup->TakeDamage(Damage);
}


void APSJ_Spaceship::ShowShield()
{
	if (!ShieldMesh)
		return;

	Multicast_ShowShield();

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ShieldAlphaTimerHandle);
		GetWorld()->GetTimerManager().SetTimer(
			ShieldAlphaTimerHandle,
			this,
			&APSJ_Spaceship::HideShield,
			ShieldDisplayDuration,
			false
		);
	}
}

void APSJ_Spaceship::HideShield()
{
	Multicast_HideShield();
}

void APSJ_Spaceship::Multicast_ShowShield_Implementation()
{
	if (ShieldMesh)
		ShieldMesh->SetHiddenInGame(false);
}

void APSJ_Spaceship::Multicast_HideShield_Implementation()
{
	if (ShieldMesh)
		ShieldMesh->SetHiddenInGame(true);
}

void APSJ_Spaceship::OnDeath()
{
	this->Destroy();
}

USpaceShipStateGroup* APSJ_Spaceship::GetSpaceShipStateGroup()
{
	if (_spaceShipStateGroup)
		return _spaceShipStateGroup;

	if (UStaticFunctionLibrary::TryGetGameState(_outGameState))
	{
		_spaceShipStateGroup = _outGameState->GetSpaceShipStateGroup();
	}
	return _spaceShipStateGroup;
}

bool APSJ_Spaceship::Server_SpaceshipBrake_Validate() { return true; }

void APSJ_Spaceship::Server_SpaceshipBrake_Implementation()
{
	if (ShipRootComponent && ShipRootComponent->IsSimulatingPhysics())
	{
		// 선형 속도(이동 속도)를 즉시 0으로 만듭니다.
		ShipRootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		
		// (선택 사항) 회전 속도도 0으로 만들어 완전히 멈추게 하려면 아래 줄을 추가하세요.
		//ShipRootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void APSJ_Spaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (IA_ThrustForward) EnhancedInputComponent->BindAction(IA_ThrustForward, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_ThrustForward);
		if (IA_ThrustBackward) EnhancedInputComponent->BindAction(IA_ThrustBackward, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_ThrustBackward);
		if (IA_MoveAxes) EnhancedInputComponent->BindAction(IA_MoveAxes, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_MoveAxes);
		if (IA_MoveUp) EnhancedInputComponent->BindAction(IA_MoveUp, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_MoveUp);
		if (IA_MouseLook) EnhancedInputComponent->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_MouseLook);
		if (IA_Roll) EnhancedInputComponent->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &APSJ_Spaceship::Input_Roll);
		if (IA_Interact) EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Started, this, &ATaskPawnBase::Input_Exit);
	}
}


void APSJ_Spaceship::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		if (APSJ_Character* Character = Cast<APSJ_Character>(OtherActor))
		{
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
			if (CurrentPilot != Character)
			{
				Character->SetCurrentSpaceship(nullptr);
			}
		}
	}
}


void APSJ_Spaceship::EnableCollisionWithPassenger(APSJ_Character* ExitedChar)
{
	if (ExitedChar && IsValid(ExitedChar))
	{
		this->MoveIgnoreActorRemove(ExitedChar);
		ExitedChar->MoveIgnoreActorRemove(this);
	}
}

void APSJ_Spaceship::OverDistanceDamageCheck()
{
	HealthComp->TakeDamage(OverDistanceDamage);
}

void APSJ_Spaceship::RegistEvent()
{
	UEventManager* _outEventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
		return;

	_eventHandleOnStartStage = _outEventManager->AddListener<UEventOnStartStage>(
		[this](UEventOnStartStage* Event)
		{
			OnStartStage(Event);
		}
	);

	_eventHandleOnEndStage = _outEventManager->AddListener<UEventOnEndStage>(
		[this](UEventOnEndStage* Event)
		{
			OnEndStage(Event);
		}
	);

	_eventHandleOnChangeMaxSpeed = _outEventManager->AddListener<UEventOnChangeSpaceShipData>(
		[this](UEventOnChangeSpaceShipData* Event)
		{
			OnChangeMexSpeed(Event);
		}
	);

	// 로비 이동 이벤트 수신 등록
	_eventHandleOnToLobby = _outEventManager->AddListener<UEventOnToLobby>(
		[this](UEventOnToLobby* Event)
		{
			OnMoveToLobby(Event);
		}
	);
}

void APSJ_Spaceship::UnregistEvent()
{
	UEventManager* _outEventManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetEventManager(_outEventManager))
		return;

	if (_eventHandleOnStartStage.IsValid())
	{
		_outEventManager->DelListener<UEventOnStartStage>(_eventHandleOnStartStage);
		_eventHandleOnStartStage.Reset();
	}

	if (_eventHandleOnEndStage.IsValid())
	{
		_outEventManager->DelListener<UEventOnEndStage>(_eventHandleOnEndStage);
		_eventHandleOnEndStage.Reset();
	}

	if (_eventHandleOnChangeMaxSpeed.IsValid())
	{
		_outEventManager->DelListener<UEventOnChangeSpaceShipData>(_eventHandleOnChangeMaxSpeed);
		_eventHandleOnChangeMaxSpeed.Reset();
	}

	if (_eventHandleOnToLobby.IsValid())
	{
		_outEventManager->DelListener<UEventOnToLobby>(_eventHandleOnToLobby);
		_eventHandleOnToLobby.Reset();
	}
}

void APSJ_Spaceship::OnStartStage(UEventOnStartStage* Event)
{
	if (Event == nullptr)
		return;

	_outGameState->SendCurrentDataEvent();
}

void APSJ_Spaceship::OnEndStage(UEventOnEndStage* Event)
{
	if (Event == nullptr)
		return;

	Server_SpaceshipBrake();
}

void APSJ_Spaceship::OnMoveToLobby(UEventOnToLobby* Event)
{
	if (Event == nullptr)
		return;

	// 권한이 있는 서버에서만 타이머를 작동시킵니다.
	if (HasAuthority())
	{
		// 2초(2.0f) 뒤에 ExecuteReturnToLobby 함수를 1회(false) 실행하도록 예약합니다.
		GetWorld()->GetTimerManager().SetTimer(
			ReturnToLobbyTimerHandle,
			this,
			&APSJ_Spaceship::ExecuteReturnToLobby,
			2.0f,
			false
		);

	}
}

// 2초 뒤에 실제로 실행될 초기화 로직
void APSJ_Spaceship::ExecuteReturnToLobby()
{
	if (ShipRootComponent && ShipRootComponent->IsSimulatingPhysics())
	{
		// 1. 선형 속도(이동)와 각속도(회전)를 완벽하게 0으로 만듭니다.
		ShipRootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ShipRootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	// 2. 우주선을 게임 시작 시 기억해둔 최초 위치로 순간이동시킵니다.
	SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);

	UE_LOG(LogTemp, Warning, TEXT("[Spaceship] 2초 지연 완료: 우주선 위치 및 속도 초기화 적용됨"));
}

void APSJ_Spaceship::OnChangeMexSpeed(UEventOnChangeSpaceShipData* Event)
{
	if (Event == nullptr)
		return;

	const FSpaceShipData& _spaceShipData = Event->SpaceShipData;
	if (_spaceShipData.DataType != E_SPACE_SHIP_DATA_TYPE::MaxSpeed)
		return;

	MaxSpeed = _spaceShipData.Data.Value.MaxValue;
}