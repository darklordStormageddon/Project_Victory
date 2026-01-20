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


APSJ_Character::APSJ_Character()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;
}

void APSJ_Character::BeginPlay()
{
	Super::BeginPlay();

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->BrakingDecelerationFlying = FlyModeBrakingDeceleration;
	GetCharacterMovement()->MaxFlySpeed = FlyModeMaxSpeed;
	GetCharacterMovement()->bImpartBaseVelocityX = false;
	GetCharacterMovement()->bImpartBaseVelocityY = false;
	GetCharacterMovement()->bImpartBaseVelocityZ = false;
	GetCharacterMovement()->bImpartBaseAngularVelocity = false;

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
				TargetShip->SetPilot(this);

				SetActorEnableCollision(false);

				// SetActorHiddenInGame(true); 

				AttachToActor(TargetShip, FAttachmentTransformRules::KeepWorldTransform);

				PC->Possess(TargetShip);

				UE_LOG(LogTemp, Warning, TEXT("=== SUCCESS: Boarded Spaceship ==="));
			}
		}
	}
}

void APSJ_Character::UpdateMagBoots(float DeltaTime)
{

	FHitResult FinalHit;
	bool bFoundValidHit = false;
	FVector Start = GetActorLocation();
	FVector End = Start + (-GetActorUpVector() * CheckDistance);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(MagBootsTraceRadius);
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
			TargetHeight += 15.0f;
		}
		float ActualDistanceToFloor = FinalHit.Distance + MagBootsTraceRadius;
		float Error = TargetHeight - ActualDistanceToFloor;
		float VerticalVelocity = FVector::DotProduct(GetVelocity(), TargetUp);
		float SpringForce = Error * SpringStiffness;
		float DampingForce = VerticalVelocity * SpringDamping;
		FVector SuspensionAccel = TargetUp * (SpringForce - DampingForce);
		GetCharacterMovement()->Velocity += SuspensionAccel * DeltaTime;
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
		if (GetCharacterMovement())
		{
			GetCharacterMovement()->SetBase(nullptr);
		}
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
		if (bIsMagBootsActive)
		{
			ForwardVector = FVector::VectorPlaneProject(ForwardVector, CurrentFloorNormal);
			ForwardVector.Normalize();
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