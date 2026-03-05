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


	ShipRootComponent = nullptr;
	PilotCamera = nullptr;
	PilotSphere = nullptr;


	DistanceComp = CreateDefaultSubobject<UDistanceComponent>(TEXT("DistanceComponent"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APSJ_Spaceship::Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot)
{
	Super::Client_BoardingSuccess_Implementation(BoardingPilot); // 부모 로직 실행


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

	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		ShipRootComponent->AddForce(GetActorForwardVector() * ThrustSpeed * Val, NAME_None, true);
	}

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
	float Val = Value.Get<float>();
	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		ShipRootComponent->AddForce(GetActorForwardVector() * -ThrustSpeed * Val, NAME_None, true);
	}
	Server_ThrustBackward(Val);
}

bool APSJ_Spaceship::Server_ThrustBackward_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_ThrustBackward_Implementation(float Value)
{
	if (!TryMove()) return;
		ShipRootComponent->AddForce(GetActorForwardVector() * -ThrustSpeed * Value, NAME_None, true);
}

void APSJ_Spaceship::Input_MoveAxes(const FInputActionValue& Value)
{
	FVector2D Val = Value.Get<FVector2D>();
	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		FVector RightForce = GetActorRightVector() * Val.X * ThrustSpeed * 0.5f;
		FVector UpForce = GetActorUpVector() * Val.Y * ThrustSpeed * 0.5f;
		ShipRootComponent->AddForce(RightForce + UpForce, NAME_None, true);
	}
	Server_MoveAxes(Val);
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
	float Val = Value.Get<float>();
	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		FVector UpForce = GetActorUpVector() * Val * ThrustSpeed * 0.5f;
		ShipRootComponent->AddForce(UpForce, NAME_None, true);
	}
	Server_MoveUp(Val);
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

	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		ShipRootComponent->AddTorqueInDegrees(GetActorForwardVector() * Val * RotateSpeed, NAME_None, true);
	}

	Server_Roll(Val);
}

bool APSJ_Spaceship::Server_Roll_Validate(float Value) { return true; }

void APSJ_Spaceship::Server_Roll_Implementation(float Value)
{
	if (!TryMove() || !ShipRootComponent) return;

	ShipRootComponent->AddTorqueInDegrees(GetActorForwardVector() * Value * RotateSpeed, NAME_None, true);
}

void APSJ_Spaceship::Input_MouseLook(const FInputActionValue& Value)
{
	FVector2D Val = Value.Get<FVector2D>();

	if (IsLocallyControlled() && ShipRootComponent && TryMove())
	{
		FVector PitchTorque = GetActorRightVector() * (Val.Y * -1.0f) * RotateSpeed;
		FVector YawTorque = GetActorUpVector() * Val.X * RotateSpeed;

		ShipRootComponent->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, true);
	}

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
		if (Mesh->GetName().Contains(TEXT("Shield")))
		{
			ShieldMesh = Mesh;
			ShieldMesh->SetHiddenInGame(true);
			ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
		}
	}


	PilotCamera = FindComponentByClass<UCameraComponent>();
	if (!PilotCamera)
	{
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


	if (DistanceComp)
		DistanceComp->OnDistanceDamaged.AddDynamic(this, &APSJ_Spaceship::OverDistanceDamageCheck);


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

	if (HasAuthority() || IsLocallyControlled())
	{
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
	}

	if (GEngine)
	{
		FVector Velocity = GetVelocity();
		float SpeedCmPerSec = Velocity.Size();
		float SpeedKmh = SpeedCmPerSec * 0.036f;




		if (LinkedChair)
		{
			float DistanceToChair = FVector::Dist(GetActorLocation(), LinkedChair->GetActorLocation());

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
		ShipRootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
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

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(
			ReturnToLobbyTimerHandle,
			this,
			&APSJ_Spaceship::ExecuteReturnToLobby,
			2.0f,
			false
		);

	}
}

void APSJ_Spaceship::ExecuteReturnToLobby()
{
	if (ShipRootComponent && ShipRootComponent->IsSimulatingPhysics())
	{
		ShipRootComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ShipRootComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);
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

void APSJ_Spaceship::Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector LocalLoc, FRotator LocalRot)
{
	Super::Client_DisembarkSuccess_Implementation(ExitingPilot, LocalLoc, LocalRot);
}