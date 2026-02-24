#include "PSJ_Spaceship.h"
#include "PSJ_ShipCockpit.h"
#include "PSJ_Character.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/GameControl/StateData/SpaceShipStateGroup.h"
#include "KSM/HealthComponent.h"
#include "JHS/UI/UIManager.h"

#include "Components/CapsuleComponent.h"

#include "JHS/SpaceObject/DriveSeatRader.h"
#include "Kismet/GameplayStatics.h"

APSJ_Spaceship::APSJ_Spaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;
	bReplicates = true;
	SetReplicateMovement(true);

	ShipRootComponent = nullptr;
	PilotCamera = nullptr;
	PilotSphere = nullptr;
	ExitPoint = nullptr;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APSJ_Spaceship::Client_BoardingSuccess_Implementation()
{
	Super::Client_BoardingSuccess_Implementation();

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

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;
	APlayerController* _pc = CurrentPilot ? Cast<APlayerController>(CurrentPilot->GetController()) : nullptr;
	if (_pc && _outUIManager)
	{
		_outUIManager->OpenUI(E_UI_TYPE::UIPanelDriveSeat);
	}
}

void APSJ_Spaceship::Input_ThrustForward(const FInputActionValue& Value)
{
	Server_ThrustForward(Value.Get<float>());
}

bool APSJ_Spaceship::Server_ThrustForward_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_ThrustForward_Implementation(float Value)
{
	if (ShipRootComponent)
		ShipRootComponent->AddForce(GetActorForwardVector() * ThrustSpeed * Value, NAME_None, true);
}

void APSJ_Spaceship::Input_ThrustBackward(const FInputActionValue& Value)
{
	Server_ThrustBackward(Value.Get<float>());
}

bool APSJ_Spaceship::Server_ThrustBackward_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_ThrustBackward_Implementation(float Value)
{
	if (ShipRootComponent)
		ShipRootComponent->AddForce(GetActorForwardVector() * -ThrustSpeed * Value, NAME_None, true);
}

void APSJ_Spaceship::Input_MoveAxes(const FInputActionValue& Value)
{
	Server_MoveAxes(Value.Get<FVector2D>());
}

bool APSJ_Spaceship::Server_MoveAxes_Validate(FVector2D Value) { return true; }

void APSJ_Spaceship::Server_MoveAxes_Implementation(FVector2D Value)
{
	if (ShipRootComponent)
	{
		FVector RightForce = GetActorRightVector() * Value.X * ThrustSpeed * 0.5f;
		FVector UpForce = GetActorUpVector() * Value.Y * ThrustSpeed * 0.5f;
		ShipRootComponent->AddForce(RightForce + UpForce, NAME_None, true);
	}
}

void APSJ_Spaceship::Input_MoveUp(const FInputActionValue& Value)
{
	Server_MoveUp(Value.Get<float>());
}

bool APSJ_Spaceship::Server_MoveUp_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_MoveUp_Implementation(float Value)
{
	if (ShipRootComponent)
	{
		FVector UpForce = GetActorUpVector() * Value * ThrustSpeed * 0.5f;
		ShipRootComponent->AddForce(UpForce, NAME_None, true);
	}
}

void APSJ_Spaceship::Input_Roll(const FInputActionValue& Value)
{
	Server_Roll(Value.Get<float>());
}

bool APSJ_Spaceship::Server_Roll_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_Roll_Implementation(float Value)
{
	AddActorLocalRotation(FRotator(0.0f, 0.0f, Value * RotateSpeed));
}

void APSJ_Spaceship::Input_MouseLook(const FInputActionValue& Value)
{
	Server_MouseLook(Value.Get<FVector2D>());
}

bool APSJ_Spaceship::Server_MouseLook_Validate(FVector2D Value) { return true; }

void APSJ_Spaceship::Server_MouseLook_Implementation(FVector2D Value)
{
	AddActorLocalRotation(FRotator(Value.Y * -1.0f, Value.X, 0.0f));
}

void APSJ_Spaceship::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnDamaged.AddDynamic(this, &APSJ_Spaceship::OnTakeDamage);
		//사망 구현(나중에 구현)
		//HealthComp->OnDeath.AddDynamic(this, &APSJ_Spaceship::OnDeath);
	}

	if (_outGameState == nullptr)
		UStaticFunctionLibrary::TryGetGameState(_outGameState);

	FTimerHandle TestDelay;
	GetWorld()->GetTimerManager().SetTimer(
		TestDelay,
		[this]() {
			UUIManager* _outUIManager = nullptr;
			if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
				return;

			_outUIManager->OpenUI(E_UI_TYPE::UIPanelDriveSeat);
		},
		1.f,
		false);

	{
		ShipRootComponent = Cast<UPrimitiveComponent>(RootComponent);
	}
	PilotCamera = FindComponentByClass<UCameraComponent>();
	if (!PilotCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Camera not found in BP"));
	}

	TArray<UArrowComponent*> Arrows;
	GetComponents(Arrows);
	for (UArrowComponent* Arrow : Arrows)
	{
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

	for (UArrowComponent* Arrow : Arrows)
	{
		if (Arrow->GetName().Contains(TEXT("Ride")))
		{
			RidePoint = Arrow;
			break;
		}
	}

	TArray<UChildActorComponent*> ChildActors;
	GetComponents(ChildActors);

	for (UChildActorComponent* ChildComp : ChildActors)
	{
		if (ChildComp && ChildComp->GetChildActor())
		{
			if (APSJ_ShipCockpit* FoundCockpit = Cast<APSJ_ShipCockpit>(ChildComp->GetChildActor()))
			{
				FoundCockpit->TargetSpaceship = this;
				LinkedCockpit = FoundCockpit;
				UE_LOG(LogTemp, Log, TEXT("Spaceship: Successfully auto-connected to Child Actor Cockpit!"));
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
}

void APSJ_Spaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// [중요] 최대 속도 제한 로직
	// ShipRootComponent가 존재하고, 물리 시뮬레이션 중이면 속도 제한 적용
	if (ShipRootComponent && ShipRootComponent->IsSimulatingPhysics())
	{
		// 1. 현재 속도 벡터를가져옴
		FVector CurrentVelocity = ShipRootComponent->GetComponentVelocity();
		float CurrentSpeed = CurrentVelocity.Size();

		// 2. 현재 속도가 최대 속도보다 크면 제한함
		if (CurrentSpeed > MaxSpeed)
		{
			// 3. 방향은 유지한 채, 크기를 MaxSpeed로 고정 (Clamping)
			FVector ClampedVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;

			// 4. 제한된 속도로 물리 속도를 직접 설정
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

		if (LinkedCockpit)
		{
			float DistanceToCockpit = FVector::Dist(GetActorLocation(), LinkedCockpit->GetActorLocation());
			FString GapMsg = FString::Printf(TEXT("[Cockpit Gap] %.2f (Is Lagging?)"), DistanceToCockpit);
			FColor GapColor = (SpeedCmPerSec > 10.0f) ? FColor::Red : FColor::Green;
			GEngine->AddOnScreenDebugMessage(3, 0.0f, GapColor, GapMsg);
		}
	}
}

void APSJ_Spaceship::OnTakeDamage(float Damage)
{
	if (GetSpaceShipStateGroup())
		_spaceShipStateGroup->DecreaseSpaceShipData(E_SPACE_SHIP_DATA_TYPE::HP, Damage);
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
		if (IA_Interact) EnhancedInputComponent->BindAction(IA_Interact, ETriggerEvent::Started, this, &APSJ_Spaceship::Input_Exit);
	}
}

void APSJ_Spaceship::SetPilot(APSJ_Character* NewPilot)
{
	CurrentPilot = NewPilot;
	if (CurrentPilot)
	{
		if (RidePoint)
		{
			CurrentPilot->SetActorEnableCollision(false);
			CurrentPilot->AttachToComponent(RidePoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			if (auto* CMC = CurrentPilot->GetCharacterMovement())
			{
				CMC->StopMovementImmediately();
				CMC->DisableMovement();
			}
		}
	}
}

void APSJ_Spaceship::DisembarkCharacter()
{
	if (!CurrentPilot) return;

	APSJ_Character* ExitingChar = CurrentPilot;
	AController* ShipController = GetController();

	CurrentPilot = nullptr;
	if (LinkedCockpit)
	{
		APlayerController* _callerPC = ExitingChar ? Cast<APlayerController>(ExitingChar->GetController()) : nullptr;
		APlayerState* _callerPS = _callerPC ? _callerPC->GetPlayerState<APlayerState>() : nullptr;
		int32 _callerPlayerId = _callerPS ? _callerPS->GetPlayerId() : -1;
		LinkedCockpit->OnInteractExit(_callerPlayerId, nullptr);
		LinkedCockpit = nullptr;
	}

	FVector SpawnLoc = ExitPoint ? ExitPoint->GetComponentLocation() : GetActorLocation();
	FRotator SpawnRot = ExitPoint ? ExitPoint->GetComponentRotation() : GetActorRotation();

	// 1. 기존 부착 해제
	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 2. 위치 설정
	ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 3. [중요함] Attach를 유지 (위치는 X)
	ExitingChar->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	// 4. 이동 모드 Custom 설정 (원래는 DisableMovement 였음)
	ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Custom);
	ExitingChar->SetReplicateMovement(false);

	// 5. 베이스 액터로 설정
	ExitingChar->SetBaseActorData(this);

	ExitingChar->SetActorEnableCollision(true);
	ExitingChar->SetActorHiddenInGame(false);

	// 6. Client RPC
	Client_DisembarkSuccess(ExitingChar, SpawnLoc, SpawnRot);

	// 7. 캐릭터 빙의
	if (ShipController)
	{
		ShipController->Possess(ExitingChar);
	}
}

void APSJ_Spaceship::Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector ExitLoc, FRotator ExitRot)
{
	if (!ExitingPilot) return;

	// 1. 먼저 움직임을 정지 (E0265 버그 해결: StopMovementImmediately 사용)
	if (UCharacterMovementComponent* CMC = ExitingPilot->GetCharacterMovement())
	{
		CMC->StopMovementImmediately(); // 속도와 가속도를 모두 0으로 초기화함.
		CMC->SetMovementMode(MOVE_Custom); // 모드를 Walking으로 바로 바꾸면 속도가 계속 남아있음
	}

	// 2. 서로 충돌 안 되게 설정
	ExitingPilot->MoveIgnoreActorRemove(this);
	this->MoveIgnoreActorRemove(ExitingPilot);

	FVector SafeExitLoc = ExitLoc + GetActorUpVector() * 15.0f;
	ExitingPilot->SetActorLocationAndRotation(SafeExitLoc, ExitRot, false, nullptr, ETeleportType::TeleportPhysics);

	// 3. 우주선에 붙인 후 하선상태 시작
	ExitingPilot->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
	ExitingPilot->StartDisembarkState();
	ExitingPilot->SetBaseActorData(this);
	ExitingPilot->ForceInputRecovery();

	UE_LOG(LogTemp, Warning, TEXT("[Disembark] Velocity Reset & Mode Set to Custom"));
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

void APSJ_Spaceship::Input_Exit(const FInputActionValue& Value)
{
	Server_RequestDisembark();
}

bool APSJ_Spaceship::Server_RequestDisembark_Validate()
{
	return true;
}

void APSJ_Spaceship::Server_RequestDisembark_Implementation()
{
	DisembarkCharacter();
}

void APSJ_Spaceship::EnableCollisionWithPassenger(APSJ_Character* ExitedChar)
{
	if (ExitedChar && IsValid(ExitedChar))
	{
		this->MoveIgnoreActorRemove(ExitedChar);
		ExitedChar->MoveIgnoreActorRemove(this);
	}
}
