#include "SpaceShip.h" // 반드시 첫 줄에 위치
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ASpaceShip::ASpaceShip()
{
    PrimaryActorTick.bCanEverTick = true;

    // 루트 컴포넌트인 구체 콜리전 생성 및 물리 설정
    RootCollision = CreateDefaultSubobject<USphereComponent>(TEXT("RootCollision"));
    SetRootComponent(RootCollision);
    RootCollision->SetSimulatePhysics(true);
    RootCollision->SetLinearDamping(0.0f);
    RootCollision->SetAngularDamping(1.0f);

    // 메시 및 카메라 설정
    ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
    ShipMesh->SetupAttachment(RootCollision);
    ShipMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(RootCollision);
}

void ASpaceShip::BeginPlay()
{
    Super::BeginPlay();

    // 플레이어 컨트롤러를 통해 Mapping Context를 등록하여 입력을 활성화합니다.
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(ShipMappingContext, 0);
        }
    }
}

void ASpaceShip::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ApplyFlightPhysics(DeltaTime);
    HandleVelocityClamp();

    // 매 프레임 입력값 초기화
    ForwardInput = 0.0f;
    RotationInput = FVector::ZeroVector;
    StrafeInput = FVector::ZeroVector;
}

void ASpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    // 입력 액션과 C++ 함수를 바인딩합니다.
    if (UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(IA_ThrustForward, ETriggerEvent::Triggered, this, &ASpaceShip::OnThrustForward);
        EIC->BindAction(IA_ThrustBackward, ETriggerEvent::Triggered, this, &ASpaceShip::OnThrustBackward);
        EIC->BindAction(IA_MoveAxes, ETriggerEvent::Triggered, this, &ASpaceShip::OnMoveAxes);
        EIC->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &ASpaceShip::OnRoll);
        EIC->BindAction(IA_MouseLook, ETriggerEvent::Triggered, this, &ASpaceShip::OnMouseLook);
    }
}

void ASpaceShip::ApplyFlightPhysics(float DeltaTime)
{
    if (GEngine)
    {
        // 모든 입력값과 델타타임을 하나의 메시지로 출력 (Key를 -1로 주어 중첩 방지)
        FString DebugStr = FString::Printf(TEXT("DT: %.4f | Thrust: %.2f | Rot: %s"),
            DeltaTime, ForwardInput, *RotationInput.ToString());
        GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, DebugStr);
    }
    // 전후진 추진력 적용
    if (FMath::Abs(ForwardInput) > 0.1f)
    {
        RootCollision->AddForce(GetActorForwardVector() * ForwardInput * ThrustStrength * DeltaTime);
        RootCollision->SetLinearDamping(0.0f);
    }
    else if (GetVelocity().Size() < StoppingThreshold)
    {
        // 추진력이 없고 저속일 때 자연스러운 정지 유도
        RootCollision->SetLinearDamping(StoppingDamping);
    }
    // 2. 좌우/상하 평행 이동 (새로 추가)
    FVector StrafeForce = (GetActorRightVector() * StrafeInput.Y) + (GetActorUpVector() * StrafeInput.Z);
    if (!StrafeForce.IsNearlyZero()) {
        // ThrustStrength를 이동 힘으로 사용하거나 별도의 StrafeStrength를 만드세요.
        RootCollision->AddForce(StrafeForce * ThrustStrength * DeltaTime);
    }

    // 6자유도 토크(회전력) 적용
    FVector Torque = GetActorForwardVector() * RotationInput.X +
        GetActorRightVector() * RotationInput.Y +
        GetActorUpVector() * RotationInput.Z;

    RootCollision->AddTorqueInDegrees(Torque * RotationSpeed * DeltaTime, NAME_None, true);
}

void ASpaceShip::HandleVelocityClamp()
{
    // 속도가 MaxSpeed를 초과하면 방향은 유지한 채 크기만 제한합니다 (Hard Clamp).
    FVector CurrentVel = RootCollision->GetPhysicsLinearVelocity();
    if (CurrentVel.Size() > MaxSpeed)
    {
        RootCollision->SetPhysicsLinearVelocity(CurrentVel.GetSafeNormal() * MaxSpeed);
    }
}

// 입력 액션 데이터로부터 로직 변수 업데이트
void ASpaceShip::OnThrustForward(const FInputActionValue& Value) { ForwardInput = 1.0f; }
void ASpaceShip::OnThrustBackward(const FInputActionValue& Value) { ForwardInput = -1.0f; }
void ASpaceShip::OnMoveAxes(const FInputActionValue& Value) {
    FVector2D AxisValue = Value.Get<FVector2D>();
    StrafeInput.Y = AxisValue.X; // 좌우 이동 (A, D)
    StrafeInput.Z = AxisValue.Y; // 상하 이동 (Up, Down 키 등)
}
void ASpaceShip::OnRoll(const FInputActionValue& Value) { RotationInput.X = Value.Get<float>(); }
void ASpaceShip::OnMouseLook(const FInputActionValue& Value) {
    FVector2D LookValue = Value.Get<FVector2D>();
    RotationInput.Y += LookValue.Y; // Mouse Pitch
    RotationInput.Z += LookValue.X; // Mouse Yaw
}