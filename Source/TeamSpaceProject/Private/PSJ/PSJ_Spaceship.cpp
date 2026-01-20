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
	ShipRootComponent = nullptr;
	PilotCamera = nullptr;
	PilotSphere = nullptr;
	ExitPoint = nullptr;
}

void APSJ_Spaceship::BeginPlay()
{
	Super::BeginPlay();

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
	DisembarkCharacter();
}

void APSJ_Spaceship::DisembarkCharacter()
{
	if (!CurrentPilot) return;

	if (!ExitPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("ExitPoint is NULL! Character will verify default location."));
	}

	APSJ_Character* ExitingChar = CurrentPilot;

	if (AController* ShipController = GetController())
	{
		ShipController->Possess(ExitingChar);
	}

	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (ExitPoint)
	{
		FVector SpawnLoc = ExitPoint->GetComponentLocation();
		FRotator SpawnRot = ExitPoint->GetComponentRotation();

		this->MoveIgnoreActorAdd(ExitingChar);
		ExitingChar->MoveIgnoreActorAdd(this);

		ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(ExitingChar->GetRootComponent()))
	{
		RootPrim->SetPhysicsLinearVelocity(FVector::ZeroVector);
		RootPrim->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
	if (ExitingChar->GetCharacterMovement())
	{
		ExitingChar->GetCharacterMovement()->Velocity = FVector::ZeroVector;
		ExitingChar->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}

	ExitingChar->SetActorHiddenInGame(false);
	ExitingChar->SetActorEnableCollision(true);

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
	float UpValue = Value.Get<float>();

	if (ShipRootComponent)
	{
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