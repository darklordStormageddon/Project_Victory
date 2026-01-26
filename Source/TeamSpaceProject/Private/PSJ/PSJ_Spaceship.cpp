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

APSJ_Spaceship::APSJ_Spaceship()
{
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.TickGroup = TG_PostPhysics;

	ShipRootComponent = nullptr;
	PilotCamera = nullptr;
	PilotSphere = nullptr;
	ExitPoint = nullptr;

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void APSJ_Spaceship::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnDamaged.AddDynamic(this, &APSJ_Spaceship::OnTakeDamage);
		HealthComp->OnDeath.AddDynamic(this, &APSJ_Spaceship::OnDeath);
	}


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
	// [추가] 이름에 'Ride'가 들어간 ArrowComponent를 찾아 RidePoint로 지정
	for (UArrowComponent* Arrow : Arrows)
	{
		if (Arrow->GetName().Contains(TEXT("Ride")))
		{
			RidePoint = Arrow;
			break;
		}
	}

	// [추가할 코드 시작] ----------------------------------------------------
	// 내 컴포넌트들 중에 ChildActorComponent를 모두 찾습니다.
	TArray<UChildActorComponent*> ChildActors;
	GetComponents(ChildActors);

	for (UChildActorComponent* ChildComp : ChildActors)
	{
		// 자식 액터가 실제로 생성되었는지 확인
		if (ChildComp && ChildComp->GetChildActor())
		{
			// 그 자식 액터가 'APSJ_ShipCockpit' 클래스인지 확인
			if (APSJ_ShipCockpit* FoundCockpit = Cast<APSJ_ShipCockpit>(ChildComp->GetChildActor()))
			{
				// 찾았다! 서로 연결해줍니다.
				FoundCockpit->TargetSpaceship = this; // 콕핏에게 "내가 대상 우주선이다" 입력

				// (선택사항) 우주선 입장에서도 이 콕핏을 알고 있으면 좋음
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

	// [디버그] 속도 및 상태 표시 로직
	if (GEngine)
	{
		// 1. 현재 속도 계산 (언리얼 단위: cm/s)
		FVector Velocity = GetVelocity();
		float SpeedCmPerSec = Velocity.Size();

		// 2. 시속으로 변환 (보기 편하게: km/h)
		// 1 cm/s = 0.036 km/h
		float SpeedKmh = SpeedCmPerSec * 0.036f;

		// 3. 화면에 출력 (Key: 1 ~ 3 번을 사용하여 줄바꿈 고정)

		// 줄 1: 현재 속도
		FString SpeedMsg = FString::Printf(TEXT("[Speed] %.2f cm/s  ( %.0f km/h )"), SpeedCmPerSec, SpeedKmh);
		GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::Yellow, SpeedMsg);

		// 줄 2: 현재 위치
		FString LocMsg = FString::Printf(TEXT("[Location] %s"), *GetActorLocation().ToString());
		GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Cyan, LocMsg);

		// 줄 3: 콕핏과의 거리 오차 (이게 0이 아니면 실제로 밀리고 있는 것임)
		if (LinkedCockpit)
		{
			// 우주선 위치 vs 콕핏 위치 거리 계산
			// (ChildActor는 부모와 위치가 같거나 고정된 오프셋이어야 함)
			// 오프셋을 고려하지 않은 단순 거리지만, 이동 중 이 값이 '변한다면' 밀리는 증거가 됨.
			float DistanceToCockpit = FVector::Dist(GetActorLocation(), LinkedCockpit->GetActorLocation());

			FString GapMsg = FString::Printf(TEXT("[Cockpit Gap] %.2f (Is Lagging?)"), DistanceToCockpit);

			// 거리가 변하면 빨간색, 고정이면 초록색
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

	AJHSGameState* _outGameState = nullptr;

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


			// [유지] 물리 고정 로직은 서버에서 해야 하므로 유지
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

// 2. Client RPC 구현: 입력과 UI는 "당사자 컴퓨터(Client)"에서 처리
void APSJ_Spaceship::Client_BoardingSuccess_Implementation()
{
	// A. 입력 매핑 컨텍스트 추가
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings(); // 캐릭터 조작 키 제거
			if (ShipMappingContext)
			{
				Subsystem->AddMappingContext(ShipMappingContext, 0); // 우주선 조작 키 추가
			}
		}
	}

	// B. UI 열기 (클라이언트 본인 화면에 뜸)
	UUIManager* _outUIManager = nullptr;
	if (UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
	{
		_outUIManager->OpenUI(E_UI_TYPE::UIPanelDriveSeat); // 콕핏 UI 열기
	}
}

// 3. Client RPC 구현: 하차 시 정리
void APSJ_Spaceship::Client_DisembarkSuccess_Implementation()
{
	// UI 닫기 등 필요한 정리 작업 수행
	// 예: UIManager->CloseUI(...) 

	// 입력 매핑은 캐릭터로 빙의(Possess)될 때 캐릭터 클래스에서 
	// 다시 SetupPlayerInputComponent가 호출되므로 여기서 굳이 안 빼도 되지만,
	// 확실하게 하려면 ClearAllMappings를 해줘도 좋습니다.
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
	// [변경] 클라이언트든 서버든 무조건 서버 RPC를 호출하여 처리
	Server_RequestDisembark();
}

// [추가] 하차 요청 RPC 구현
bool APSJ_Spaceship::Server_RequestDisembark_Validate()
{
	return true;
}

void APSJ_Spaceship::Server_RequestDisembark_Implementation()
{
	// 서버에서 실제 하차 로직 수행
	DisembarkCharacter();
}

void APSJ_Spaceship::DisembarkCharacter()
{
	// 1. 안전 검사
	if (!CurrentPilot) return;

	APSJ_Character* ExitingChar = CurrentPilot;

	// 2. 컨트롤러 제어권 반환
	if (AController* ShipController = GetController())
	{
		// 내리기 직전에 클라이언트 정리 RPC 호출
		Client_DisembarkSuccess();

		ShipController->Possess(ExitingChar);
	}

	// 3. 우주선에서 분리
	ExitingChar->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	if (ExitPoint)
	{
		// [수정] 보정 공식 삭제! -> ExitPoint의 현재 위치를 그대로 사용합니다.
		// 테스트하신 큐브처럼 콕핏이 잘 따라온다면 이 좌표가 정확합니다.
		FVector SpawnLoc = ExitPoint->GetComponentLocation();
		FRotator SpawnRot = ExitPoint->GetComponentRotation();

		// 위치 이동
		ExitingChar->SetActorLocationAndRotation(SpawnLoc, SpawnRot, false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ExitPoint is NULL!"));
	}

	// 4. 물리 및 이동 상태 초기화
	if (auto* CMC = ExitingChar->GetCharacterMovement())
	{
		// [옵션] 관성 상속 (자연스러운 하차를 위해 속도는 유지)
		// 만약 캐릭터가 내리자마자 뚝 멈추길 원하시면 아래 줄을 지우고
		// CMC->Velocity = FVector::ZeroVector; 로 바꾸세요.
		CMC->Velocity = this->GetVelocity();

		// 공중 상태 설정 (바닥에 닿으면 걸음)
		CMC->SetMovementMode(MOVE_Falling);
	}

	// 5. 충돌 켜기
	ExitingChar->SetActorHiddenInGame(false);
	ExitingChar->SetActorEnableCollision(true);

	// 6. 정리
	if (LinkedCockpit)
	{
		LinkedCockpit->OnInteractExit(nullptr);
		LinkedCockpit = nullptr;
	}

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