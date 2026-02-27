// Fill out your copyright notice in the Description page of Project Settings.


#include "YSH/TurretBase_GT.h"
#include "YSH/Projectile.h"

#include "PSJ/PSJ_Character.h"     // PSJ 폴더 안에 있는 캐릭터 헤더
#include "PSJ/TaskChair.h"   // PSJ 폴더 안에 있는 콕핏 헤더
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

	// 카메라 설정 - BarrelMesh에 직접 부착
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(BarrelMesh);  // PitchPivot에서 BarrelMesh로 변경
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	SpringArm->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = true;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = true;

	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;

	// SpringArm->bUseCameraLagSubstepping = true;
	// SpringArm->CameraLagMaxDistance = 0.0f;  // 최대 지연 거리를 0으로 설정

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

	// 카메라는 SpringArm이 BarrelMesh에 부착되어 있으므로 자동으로 포신을 따라감
	// 추가적인 카메라 회전 로직 제거됨

	// 연속 발사 로직
	if (bIsFiring && _cachedGameState)
	{
		TimeSinceLastFire += DeltaTime;

		float CurrentFireCoolTime = 0.0f;
		UTurretStateGroup* TurretStateGroup = _cachedGameState->GetTurretStateGroup();

		if (TurretStateGroup && TurretStateGroup->TryGetTurretFireInterval(TurretPosition, &CurrentFireCoolTime))
		{
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
			}
		}
	}

	// Pitch (상하 회전) - Y축에 음수 적용
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
	if (MainMuzzle && ProjectileClass && Camera)
	{
		// 카메라 중앙에서 레이캐스트로 조준점 찾기
		FVector CameraLocation = Camera->GetComponentLocation();
		FVector CameraForward = Camera->GetForwardVector();

		// 레이캐스트 최대 거리
		float TraceDistance = 10000.0f;
		FVector TraceEnd = CameraLocation + (CameraForward * TraceDistance);

		// 충돌 검사 설정
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		if (CurrentPilot)
		{
			QueryParams.AddIgnoredActor(CurrentPilot);
		}

		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			CameraLocation,
			TraceEnd,
			ECC_Visibility,
			QueryParams
		);

		// 조준점 결정 (충돌하면 충돌 지점, 아니면 최대 거리 지점)
		FVector TargetPoint = bHit ? HitResult.ImpactPoint : TraceEnd;

		// 머즐 위치와 회전
		FVector MuzzleLocation = MainMuzzle->GetComponentLocation();

		// 머즐에서 조준점을 향하는 방향 계산
		FVector FireDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
		FRotator FireRotation = FireDirection.Rotation();

		// 이펙트용 회전 및 위치 계산 (머즐 기준)
		FRotator EffectRotation = FireRotation + MuzzleFlashRotationOffset;
		FVector EffectLocation = MuzzleLocation + FireRotation.RotateVector(MuzzleFlashLocationOffset);

		// 발사체 스폰 (카메라 조준 방향으로)
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
			ProjectileClass,
			MuzzleLocation,
			FireRotation,  // ← 카메라 조준 방향 사용
			SpawnParams
		);

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
void ATurretBase_GT::SetPilot(APSJ_Character* NewPilot, ATaskChair* Chair)
{
	CurrentPilot = NewPilot;
	LinkedSeat = Chair;

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
void ATurretBase_GT::Client_BoardingSuccess_Implementation(APSJ_Character* BoardingPilot)
{
	// 탑승 시 포탑 회전 초기화
	if (YawPivot)
	{
		FRotator ResetYaw = YawPivot->GetRelativeRotation();
		ResetYaw.Yaw = 0.0f;  // Yaw를 0도로 초기화 (정면)
		YawPivot->SetRelativeRotation(ResetYaw);
		TargetYaw = 0.0f;
	}

	if (PitchPivot)
	{
		FRotator ResetPitch = PitchPivot->GetRelativeRotation();
		ResetPitch.Roll = 0.0f;  // Roll을 0으로 초기화 (수평)
		PitchPivot->SetRelativeRotation(ResetPitch);
		TargetPitchRoll = 0.0f;
	}

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

//// [신규] 하차 요청 (입력 시 호출)
//void ATurretBase_GT::Input_Exit(const FInputActionValue& Value)
//{
//	Server_RequestDisembark();
//}

void ATurretBase_GT::DisembarkCharacter()
{

	if (!CurrentPilot) return;

	if (LinkedSeat)
	{
		APlayerController* CallerPC = Cast<APlayerController>(CurrentPilot->GetController());

		int32 CallerPlayerId = -1;
		if (CallerPC && CallerPC->PlayerState)
		{
			CallerPlayerId = CallerPC->PlayerState->GetPlayerId();
		}

		if (ATaskChair* Chair = Cast<ATaskChair>(LinkedSeat))
		{
			Chair->OnInteractExit(CallerPlayerId, nullptr);
		}
		LinkedSeat = nullptr;
	}

	Super::DisembarkCharacter();
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