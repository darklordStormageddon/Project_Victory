// Fill out your copyright notice in the Description page of Project Settings.


#include "YSH/TurretBase_GT.h"
#include "YSH/Projectile.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Enhanced Input 헤더
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"

#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"

ATurretBase_GT::ATurretBase_GT()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	YawPivot = CreateDefaultSubobject<USceneComponent>(TEXT("YawPivot"));
	YawPivot->SetupAttachment(Root);

	PitchPivot = CreateDefaultSubobject<USceneComponent>(TEXT("PitchPivot"));
	PitchPivot->SetupAttachment(YawPivot);

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(YawPivot);

	BarrelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrelMesh"));
	BarrelMesh->SetupAttachment(PitchPivot);

	// 머즐 컴포넌트 생성
	MainMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("MainMuzzle"));
	MainMuzzle->SetupAttachment(BarrelMesh);

	// 카메라 설정
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(PitchPivot);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	SpringArm->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false; // Yaw 수동 조작
	SpringArm->bInheritRoll = false;

	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraRotationLagSpeed = 10.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Pawn으로 자동 빙의 설정
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ATurretBase_GT::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 입력을 위한 설정
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());

		// Enhanced Input Subsystem에 포탑 전용 IMC 추가
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (TurretMappingContext)
			{
				Subsystem->AddMappingContext(TurretMappingContext, MappingPriority);
				UE_LOG(LogTemp, Warning, TEXT("TurretBase: Turret Mapping Context added with priority %d"), MappingPriority);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TurretBase: TurretMappingContext is not set!"));
			}
		}
	}

	// 초기 회전 값 저장
	if (SpringArm)
	{
		FRotator InitialRotation = SpringArm->GetRelativeRotation();
		InitialSpringArmRoll = InitialRotation.Roll;
		CurrentCameraYaw = InitialRotation.Yaw;
	}

	if (YawPivot)
	{
		TargetYaw = YawPivot->GetRelativeRotation().Yaw;
	}

	if (PitchPivot)
	{
		TargetPitchRoll = PitchPivot->GetRelativeRotation().Roll;
	}
}

void ATurretBase_GT::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// IMC 제거 (포탑에서 내릴 때)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (TurretMappingContext)
			{
				Subsystem->RemoveMappingContext(TurretMappingContext);
				UE_LOG(LogTemp, Warning, TEXT("TurretBase: Turret Mapping Context removed"));
			}
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ATurretBase_GT::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bEnableBarrelLag)
	{
		// Yaw (좌우 회전)
		if (YawPivot)
		{
			FRotator CurrentYawRotation = YawPivot->GetRelativeRotation();
			CurrentYawRotation.Yaw = FMath::FInterpTo(
				CurrentYawRotation.Yaw,
				TargetYaw,
				DeltaTime,
				BarrelFollowSpeed
			);
			YawPivot->SetRelativeRotation(CurrentYawRotation);
		}

		// Pitch/Roll (상하 회전)
		if (PitchPivot)
		{
			FRotator CurrentPitchRotation = PitchPivot->GetRelativeRotation();
			CurrentPitchRotation.Roll = FMath::FInterpTo(
				CurrentPitchRotation.Roll,
				TargetPitchRoll,
				DeltaTime,
				BarrelFollowSpeed
			);
			PitchPivot->SetRelativeRotation(CurrentPitchRotation);
		}
	}

	if (SpringArm)
	{
		FRotator CurrentSpringArmRotation = SpringArm->GetRelativeRotation();

		// Yaw 업데이트
		if (bSmoothCameraFollow)
		{
			CurrentCameraYaw = FMath::FInterpTo(
				CurrentCameraYaw,
				TargetYaw,
				DeltaTime,
				CameraYawFollowSpeed
			);
		}
		else
		{
			CurrentCameraYaw = TargetYaw;
		}
		CurrentSpringArmRotation.Yaw = CurrentCameraYaw;

		// Roll 업데이트
		float CameraTargetRoll = InitialSpringArmRoll + (TargetPitchRoll * CameraPitchFollowRatio);

		if (bSmoothCameraFollow)
		{
			CurrentSpringArmRotation.Roll = FMath::FInterpTo(
				CurrentSpringArmRotation.Roll,
				CameraTargetRoll,
				DeltaTime,
				CameraPitchFollowSpeed
			);
		}
		else
		{
			CurrentSpringArmRotation.Roll = CameraTargetRoll;
		}

		SpringArm->SetRelativeRotation(CurrentSpringArmRotation);
	}

	// 연속 발사 로직
	if (bIsFiring)
	{
		TimeSinceLastFire += DeltaTime;

		float CurrentFireRate = GetCurrentFireRate();
		if (TimeSinceLastFire >= CurrentFireRate)
		{
			TryFire();
			TimeSinceLastFire -= CurrentFireRate; // 정확한 타이밍을 위해 -= 사용
		}
	}
}

void ATurretBase_GT::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATurretBase_GT::Look);
		}

		// Fire 바인딩 수정 - Started만 사용
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATurretBase_GT::Fire);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATurretBase_GT::StopFire);
		}
	}
}

void ATurretBase_GT::Look(const FInputActionValue& Value)
{
	// FVector2D로 마우스 X, Y 값 받기
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// Yaw (좌우 회전)
	if (FMath::Abs(LookAxisVector.X) > 0.01f)
	{
		if (bEnableBarrelLag)
		{
			TargetYaw += LookAxisVector.X * TurretYawSpeed * DeltaTime;
		}
		else
		{
			if (YawPivot)
			{
				FRotator CurrentRotation = YawPivot->GetRelativeRotation();
				CurrentRotation.Yaw += LookAxisVector.X * TurretYawSpeed * DeltaTime;
				YawPivot->SetRelativeRotation(CurrentRotation);
				TargetYaw = CurrentRotation.Yaw;
				CurrentCameraYaw = TargetYaw;
			}
		}
	}

	// Pitch (상하 회전)
	if (FMath::Abs(LookAxisVector.Y) > 0.01f)
	{
		if (bEnableBarrelLag)
		{
			float NewTargetRoll = FMath::Clamp(
				TargetPitchRoll + (-LookAxisVector.Y * TurretPitchSpeed * DeltaTime),
				MinPitch,
				MaxPitch
			);
			TargetPitchRoll = NewTargetRoll;
		}
		else
		{
			if (PitchPivot)
			{
				FRotator CurrentRotation = PitchPivot->GetRelativeRotation();
				float NewRoll = FMath::Clamp(
					CurrentRotation.Roll + (-LookAxisVector.Y * TurretPitchSpeed * DeltaTime),
					MinPitch,
					MaxPitch
				);
				CurrentRotation.Roll = NewRoll;
				PitchPivot->SetRelativeRotation(CurrentRotation);
				TargetPitchRoll = NewRoll;
			}
		}
	}
}

void ATurretBase_GT::Fire(const FInputActionValue& Value)
{
	if (!bIsFiring)
	{
		bIsFiring = true;
		TryFire(); // 첫 발사는 즉시
		TimeSinceLastFire = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("Fire!"));
	}
}

void ATurretBase_GT::StopFire(const FInputActionValue& Value)
{
	bIsFiring = false;
	TimeSinceLastFire = 0.0f;
}

void ATurretBase_GT::TryFire()
{
	if (MainMuzzle && ProjectileClass)
	{
		// 기본 위치와 회전
		FVector MuzzleLocation = MainMuzzle->GetComponentLocation();
		FRotator MuzzleRotation = MainMuzzle->GetComponentRotation();

		//이펙트 회전위치 계산
		FRotator EffectRotation = MuzzleRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + MuzzleRotation.RotateVector(MuzzleFlashLocationOffset);

		// 발사체 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);

		bIsLeftMuzzleNext = !bIsLeftMuzzleNext;

		// Cascade 이펙트
		if (MuzzleFlashEffect)
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlashEffect,
				EffectLocation,           
				EffectRotation,           
				FVector(MuzzleFlashScale),
				true,
				EPSCPoolMethod::AutoRelease,
				true
			);

			if (PSC)
			{
				PSC->bAutoDestroy = true;
				PSC->SecondsBeforeInactive = 0.0f;
			}
		}

		// 사운드
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, EffectLocation);
		}

		// 카메라 쉐이크
		if (FireCameraShake)
		{
			APlayerController* PC = Cast<APlayerController>(GetController());
			if (PC)
			{
				PC->ClientStartCameraShake(FireCameraShake);
			}
		}
	}
}



void ATurretBase_GT::AddYawInput(float YawInputDegPerSec, float DeltaTime)
{
	if (YawPivot)
	{
		if (bEnableBarrelLag)
		{
			TargetYaw += YawInputDegPerSec * DeltaTime;
		}
		else
		{
			FRotator R = YawPivot->GetRelativeRotation();
			R.Yaw += YawInputDegPerSec * DeltaTime;
			YawPivot->SetRelativeRotation(R);
			TargetYaw = R.Yaw;
			CurrentCameraYaw = TargetYaw;
		}
	}
}

void ATurretBase_GT::AddPitchInput(float PitchInputDegPerSec, float DeltaTime)
{
	if (PitchPivot)
	{
		if (bEnableBarrelLag)
		{
			float NewTargetRoll = FMath::Clamp(
				TargetPitchRoll + PitchInputDegPerSec * DeltaTime,
				MinPitch,
				MaxPitch
			);
			TargetPitchRoll = NewTargetRoll;
		}
		else
		{
			FRotator R = PitchPivot->GetRelativeRotation();
			float NewRoll = FMath::Clamp(R.Roll + PitchInputDegPerSec * DeltaTime, MinPitch, MaxPitch);
			R.Roll = NewRoll;
			PitchPivot->SetRelativeRotation(R);
			TargetPitchRoll = NewRoll;
		}
	}
}