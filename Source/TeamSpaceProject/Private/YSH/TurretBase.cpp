// Fill out your copyright notice in the Description page of Project Settings.


#include "YSH/TurretBase.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
ATurretBase::ATurretBase()
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

	// 카메라 설정 - YawPivot에 붙여서 수평 회전을 따라가도록 함
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(YawPivot);
	SpringArm->TargetArmLength = 400.0f;
	SpringArm->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f)); 
	SpringArm->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f)); 

	// 상속 설정 - YawPivot의 회전을 따라가도록
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = true;   
	SpringArm->bInheritRoll = false;

	// 부드러운 카메라 움직임 (선택사항)
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 3.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 10.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// Pawn으로 자동 빙의 설정
	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void ATurretBase::BeginPlay()
{
	Super::BeginPlay();

	// 마우스 입력을 위한 설정
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC)
	{
		// 마우스 커서 숨기기 및 입력 모드 설정
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ATurretBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATurretBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 직접 마우스 축 바인딩
	PlayerInputComponent->BindAxis("MouseX", this, &ATurretBase::LookYaw);
	PlayerInputComponent->BindAxis("MouseY", this, &ATurretBase::LookPitch);
}

void ATurretBase::LookYaw(float Value)
{
	if (FMath::Abs(Value) > 0.01f)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		FRotator CurrentRotation = YawPivot->GetRelativeRotation();
		CurrentRotation.Yaw += Value * TurretYawSpeed * DeltaTime;
		YawPivot->SetRelativeRotation(CurrentRotation);
	}
}

void ATurretBase::LookPitch(float Value)
{
	if (FMath::Abs(Value) > 0.01f)
	{
		float DeltaTime = GetWorld()->GetDeltaSeconds();
		FRotator CurrentRotation = PitchPivot->GetRelativeRotation();

		float NewRoll = FMath::Clamp(
			CurrentRotation.Roll + (-Value * TurretPitchSpeed * DeltaTime),
			MinPitch,
			MaxPitch
		);
		CurrentRotation.Roll = NewRoll;
		PitchPivot->SetRelativeRotation(CurrentRotation);
	}
}

void ATurretBase::AddYawInput(float YawInputDegPerSec, float DeltaTime)
{
	FRotator R = YawPivot->GetRelativeRotation();
	R.Yaw += YawInputDegPerSec * DeltaTime;
	YawPivot->SetRelativeRotation(R);
}

void ATurretBase::AddPitchInput(float PitchInputDegPerSec, float DeltaTime)
{
	FRotator R = PitchPivot->GetRelativeRotation();
	float NewRoll = FMath::Clamp(R.Roll + PitchInputDegPerSec * DeltaTime, MinPitch, MaxPitch);
	R.Roll = NewRoll;
	PitchPivot->SetRelativeRotation(R);
}