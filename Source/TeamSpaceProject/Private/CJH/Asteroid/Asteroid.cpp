#include "CJH/Asteroid/Asteroid.h"
#include "CJH/Asteroid/AsteroidComponent.h"

#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/SpaceObject/SpaceRader.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"
#include "JHS/SpaceObject/SpaceObjectManager.h"


// Sets default values

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	SpaceObjectComp = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	SetAsteroidRot();

	HealthComp->OnDeath.AddDynamic(this, &AAsteroid::OnDestroy);
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AJHSGameMode* InGameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());
	if (!InGameMode)
		return;

	if (!HasAuthority())
		return;
	
	MoveAsteroid(DeltaTime);

	float DestroyDist = FVector::Dist(InGameMode->GetSpaceStation()->GetActorLocation(), GetActorLocation());
	
	if (DestroyDist > InGameMode->GetSpaceRadius() - 1)
		Destroy();
}

void AAsteroid::SetAsteroidInfo(
	const FTargetInfo& InAsteroidInfo,
	FVector VSpaceShip,
	FVector Velocity)
{
	_targetInfo = InAsteroidInfo;

	//운석의 크기 설정
	SetActorScale3D(FVector(_targetInfo.Size));

	bool bHasIntercept = CalculateInterceptPoint(
		GetActorLocation(),
		VSpaceShip,
		Velocity
	);

	if (!bHasIntercept)
		TargetLocation = VSpaceShip;
	
	// === 방향 계산 ===
	Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();// 방향 벡터 단위벡터화
	Direction *= _targetInfo.Speed;

	AJHSGameMode* GameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());

	HealthComp->SetCurrentHP(_targetInfo.Max_HP);


}

bool AAsteroid::CalculateInterceptPoint(
	const FVector& AsteroidPos,
	const FVector& ShipPos,
	const FVector& ShipVelocity
)
{
	FVector R = ShipPos - AsteroidPos;
	FVector V = ShipVelocity;

	float a = FVector::DotProduct(V, V) - _targetInfo.Speed * _targetInfo.Speed;
	float b = 2.f * FVector::DotProduct(R, V);
	float c = FVector::DotProduct(R, R);

	float Discriminant = b * b - 4.f * a * c;

	if (Discriminant < 0.f)
		return false;

	float sqrtD = FMath::Sqrt(Discriminant);

	float t1 = (-b - sqrtD) / (2.f * a);
	float t2 = (-b + sqrtD) / (2.f * a);

	float t = TNumericLimits<float>::Max();

	if (t1 > 0.f) t = t1;
	if (t2 > 0.f && t2 < t) t = t2;

	if (t == TNumericLimits<float>::Max())
		return false;

	TargetLocation = ShipPos + ShipVelocity * t;
	return true;
}

void AAsteroid::SetAsteroidRot()
{
	ConstRotaion = FRotator(
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(-1.0f, 1.0f));

	RotateSpeed = FMath::RandRange(MinRotateSpeed, MaxRotateSpeed);
}

void AAsteroid::MoveAsteroid(float DeltaTime)
{
	MoveDistance += Direction.Size() * DeltaTime;

	AddActorWorldOffset(Direction * DeltaTime, true);
	AddActorWorldRotation(ConstRotaion * RotateSpeed * DeltaTime);
}

void AAsteroid::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if(AsteroidComponent)
		AsteroidComponent->RemoveAsteroid(this);

	SpaceObject_Remove();
	 
	Super::EndPlay(EndPlayReason);
}

void AAsteroid::SpaceObject_Remove()
{
	USpaceObjectManager* _spaceManager = nullptr;

	if (UStaticFunctionLibrary::TryGetSpaceObjectManager(_spaceManager))
	{
		_spaceManager->RemoveSpaceObject(SpaceObjectComp);
	}
}

void AAsteroid::DebugDrawing()
{
	DrawDebugSphere(
		GetWorld(),
		TargetLocation,
		50.f,
		16,
		FColor::Red,
		false,
		5.f
	);
}

void AAsteroid::OnDestroy()
{
	this->Destroy();
}