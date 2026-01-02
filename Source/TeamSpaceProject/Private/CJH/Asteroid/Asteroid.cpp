#include "CJH/Asteroid/Asteroid.h"
#include "CJH/Asteroid/AsteroidComponent.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/SpaceObject/SpaceRader.h"
// Sets default values

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	SetAsteroidRot();
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AJHSGameMode* InGameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());
	if (!InGameMode)
		return;

	MoveAsteroid(DeltaTime);
	float DestroyDist = FVector::Dist(InGameMode->GetSpaceStation()->GetActorLocation(), GetActorLocation());

	if (DestroyDist > DestroyDistance)
		DestroyAsteroid();
}

void AAsteroid::SetAsteroidInfo(
	const FAsteroidInfo& InAsteroidInfo,
	FVector VSpaceShip,
	FVector Velocity)
{
	AsteroidInfo = InAsteroidInfo;

	//운석의 크기 설정
	SetActorScale3D(FVector(AsteroidInfo.Size));

	//운석의 속도에 따라 우주선 방향과 속도가 더해진 벡터로 운석의 이동 방향이 정해짐
	Direction = VSpaceShip - GetActorLocation() + (Velocity * Direction.Size());

	Direction.Normalize();

	Direction *= AsteroidInfo.Speed;
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

void AAsteroid::DestroyAsteroid()
{
	this->Destroy();
}