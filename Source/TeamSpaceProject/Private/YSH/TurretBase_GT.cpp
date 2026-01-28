// Fill out your copyright notice in the Description page of Project Settings.


#include "YSH/TurretBase_GT.h"
#include "YSH/Projectile.h"

#include "PSJ/PSJ_Character.h"     // PSJ 폴더 안에 있는 캐릭터 헤더
#include "PSJ/PSJ_ShipCockpit.h"   // PSJ 폴더 안에 있는 콕핏 헤더
#include "GameFramework/CharacterMovementComponent.h" // 무브먼트 제어용

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameState.h"
#include "JHS/UI/UIManager.h"
#include "JHS/GameControl/StateData/TurretStateGroup.h"

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
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraRotationLagSpeed = 10.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Pawn으로 자동 빙의 설정 -> 멀티플레이어를 위해 해제
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ATurretBase_GT::BeginPlay()
{
	Super::BeginPlay();

	// GameState 캐싱
	AJHSGameState* TempGameState = nullptr;
	if (UStaticFunctionLibrary::TryGetGameState(TempGameState))
	{
		_cachedGameState = TempGameState;
		//_cachedGameState->GetTurretStateGroup()->SetInfiniteMagMode(true);
		//_cachedGameState->GetTurretStateGroup()->TryEquipTurret(E_TURRET_POSITION::Main, E_AMMO_TYPE::Bullet, this);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase_GT: Failed to get GameState"));
	}

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
				UE_LOG(LogTemp, Warning, TEXT("TurretBase_GT: Turret Mapping Context added with priority %d"), MappingPriority);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("TurretBase_GT: TurretMappingContext is not set!"));
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

	//UI 연동 테스트 Pawn 스위칭 연동 후 삭제
	UUIManager* TempUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(TempUIManager))
	{
		return;
	}

	//TempUIManager->OpenUI(E_UI_TYPE::UIPanelTurretSeat);
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
				UE_LOG(LogTemp, Warning, TEXT("TurretBase_GT: Turret Mapping Context removed"));
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
			// 최단 경로로 보간하기 위해 FInterpTo 대신 직접 계산
			float CurrentYaw = CurrentYawRotation.Yaw;
			float DeltaYaw = FRotator::NormalizeAxis(TargetYaw - CurrentYaw);
			CurrentYawRotation.Yaw = CurrentYaw + (DeltaYaw * FMath::Min(1.0f, DeltaTime * BarrelFollowSpeed));
			CurrentYawRotation.Normalize();
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
			// 최단 경로로 보간
			float DeltaYaw = FRotator::NormalizeAxis(TargetYaw - CurrentCameraYaw);
			CurrentCameraYaw = CurrentCameraYaw + (DeltaYaw * FMath::Min(1.0f, DeltaTime * CameraYawFollowSpeed));
			CurrentCameraYaw = FRotator::NormalizeAxis(CurrentCameraYaw);
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

	// 연속 발사 로직 - GameState 기반으로 수정
	if (bIsFiring && _cachedGameState)
	{
		TimeSinceLastFire += DeltaTime;

		float CurrentFireCoolTime = 0.0f;
		UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();

		if (TurretStateGroup && TurretStateGroup->TryGetTurretFireInterval(TurretPosition, &CurrentFireCoolTime))
		{
			// FireRateMultiplier 적용
			float AdjustedFireCoolTime = CurrentFireCoolTime / FireRateMultiplier;

			if (TimeSinceLastFire >= AdjustedFireCoolTime)
			{
				TryFire();
				TimeSinceLastFire -= AdjustedFireCoolTime;
			}
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

		// [추가] 내리기(Exit) 바인딩
		if (InteractAction)
		{
			EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ATurretBase_GT::Input_Exit);
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
			// 각도 정규화: -180 ~ 180 범위로 제한
			TargetYaw = FRotator::NormalizeAxis(TargetYaw);
		}
		else
		{
			if (YawPivot)
			{
				FRotator CurrentRotation = YawPivot->GetRelativeRotation();
				CurrentRotation.Yaw += LookAxisVector.X * TurretYawSpeed * DeltaTime;
				// 각도 정규화
				CurrentRotation.Normalize();
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
	}
}

void ATurretBase_GT::StopFire(const FInputActionValue& Value)
{
	bIsFiring = false;
	TimeSinceLastFire = 0.0f;
}

void ATurretBase_GT::TryFire()
{
	UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();
	if (!TurretStateGroup)
	{
		UE_LOG(LogTemp, Error, TEXT("ATurretBase_GT::TryFire - TurretStateGroup is null"));
		return;
	}

	// TryFireTurret으로 탄약 소비 및 발사 가능 여부 확인
	if (!TurretStateGroup->TryFireTurret(TurretPosition))
	{
		// 발사 실패 (탄약 부족 등)
		return;
	}

	// 발사 성공 - 실제 발사 로직 실행
	if (MainMuzzle && ProjectileClass)
	{
		// 기본 위치와 회전
		FVector MuzzleLocation = MainMuzzle->GetComponentLocation();
		FRotator MuzzleRotation = MainMuzzle->GetComponentRotation();

		// ↓↓↓ 이펙트용 회전 및 위치 계산 ↓↓↓
		FRotator EffectRotation = MuzzleRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + MuzzleRotation.RotateVector(MuzzleFlashLocationOffset);

		// 발사체 스폰
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);

		bIsLeftMuzzleNext = !bIsLeftMuzzleNext;

		// Cascade 이펙트 (보정된 회전 사용)
		if (MuzzleFlashEffect)
		{
			UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlashEffect,
				EffectLocation,           // ← 오프셋 적용된 위치
				EffectRotation,           // ← 보정된 회전 (90도 조정)
				FVector(MuzzleFlashScale), // ← 크기
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

// [신규] 탑승 설정 (서버에서 실행)
void ATurretBase_GT::SetPilot(APSJ_Character* NewPilot, APSJ_ShipCockpit* Cockpit)
{
	CurrentPilot = NewPilot;
	LinkedCockpit = Cockpit;

	if (CurrentPilot)
	{
		// 1. 캐릭터 충돌 끄고 숨기기 (또는 의자에 앉히기)
		CurrentPilot->SetActorEnableCollision(false);

		// 터렛 위치 혹은 의자 위치로 이동 (여기서는 터렛 Root에 붙임, 필요 시 소켓 지정 가능)
		CurrentPilot->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		if (auto* CMC = CurrentPilot->GetCharacterMovement())
		{
			CMC->StopMovementImmediately();
			CMC->DisableMovement();
		}
	}
}

// [신규] 클라이언트 탑승 성공 처리 (UI, IMC)
void ATurretBase_GT::Client_BoardingSuccess_Implementation()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// 기존 매핑 싹 비우고 터렛 전용 매핑 추가
			Subsystem->ClearAllMappings();
			if (TurretMappingContext)
			{
				Subsystem->AddMappingContext(TurretMappingContext, MappingPriority);
			}
		}
	}

	// UI 열기 (UIPanelTurretSeat)
	UUIManager* TempUIManager = nullptr;
	if (UStaticFunctionLibrary::TryGetUIManager(TempUIManager))
	{
		TempUIManager->OpenUI(E_UI_TYPE::UIPanelTurretSeat);
	}
}

// [신규] 하차 요청 (입력 시 호출)
void ATurretBase_GT::Input_Exit(const FInputActionValue& Value)
{
	Server_RequestDisembark();
}

bool ATurretBase_GT::Server_RequestDisembark_Validate() { return true; }

void ATurretBase_GT::Server_RequestDisembark_Implementation()
{
	DisembarkCharacter();
}

// [신규] 하차 로직 구현
void ATurretBase_GT::DisembarkCharacter()
{
	if (!CurrentPilot) return;

	APSJ_Character* ExitingChar = CurrentPilot;
	AController* TurretController = GetController();

	CurrentPilot = nullptr;

	// 연결된 콕핏에 하차 알림 (필요하다면)
	if (LinkedCockpit)
	{
		LinkedCockpit->OnInteractExit(nullptr);
		LinkedCockpit = nullptr;
	}

	// 하차 위치 계산 (콕핏 앞이나 터렛 주변, 여기서는 임시로 현재 위치)
	FVector SpawnLoc = GetActorLocation() + (GetActorRightVector() * 200.0f); // 우측 하차 예시
	FRotator SpawnRot = FRotator(0.0f, GetActorRotation().Yaw, 0.0f);

	// 1. 부착 해제
	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	// 2. 위치 이동 및 물리 복구
	ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot);
	ExitingChar->SetActorEnableCollision(true);
	ExitingChar->SetActorHiddenInGame(false);

	if (auto* CMC = ExitingChar->GetCharacterMovement())
	{
		CMC->SetMovementMode(MOVE_Falling); // 혹은 MOVE_Walking
	}

	// 3. Client RPC로 정리 지시
	Client_DisembarkSuccess(ExitingChar, SpawnLoc, SpawnRot);

	// 4. 제어권 반환 (빙의)
	if (TurretController)
	{
		TurretController->Possess(ExitingChar);
	}
}

// [신규] 클라이언트 하차 후처리
void ATurretBase_GT::Client_DisembarkSuccess_Implementation(APSJ_Character* ExitingPilot, FVector ExitLoc, FRotator ExitRot)
{
	if (!ExitingPilot) return;

	// 캐릭터의 입력 복구 함수 호출 (PSJ_Spaceship에 구현된 것과 동일한 원리)
	ExitingPilot->ForceInputRecovery();

	// 2. [추가] 터렛 UI 닫기
		// UI 매니저를 불러와서 열려있는 터렛 UI를 닫습니다.
	UUIManager* TempUIManager = nullptr;
	if (UStaticFunctionLibrary::TryGetUIManager(TempUIManager))
	{
		// CloseUI 함수가 있고, 같은 Enum을 쓴다고 가정합니다.
		// 만약 함수 이름이 ClosePanel 이거나 HideUI라면 그에 맞춰 수정해주세요.
		TempUIManager->CloseUI(E_UI_TYPE::UIPanelTurretSeat);
	}

	// 터렛 매핑 컨텍스트 제거
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				if (TurretMappingContext)
					Subsystem->RemoveMappingContext(TurretMappingContext);
			}
		}
	}
}