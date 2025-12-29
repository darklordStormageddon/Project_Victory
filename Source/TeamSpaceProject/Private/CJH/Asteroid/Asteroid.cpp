#include "CJH/Asteroid/Asteroid.h"

// Sets default values
// Sets default values
AAsteroid::AAsteroid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(GetActorLocation() + Direction * DeltaTime);
}

void AAsteroid::SetMeteorInfo(
	const FMeteorInfo& InMeteorInfo,
	FVector VSpaceShip,
	FVector Velocity)
{
	MeteorInfo = InMeteorInfo;

	//운석의 크기 설정
	SetActorScale3D(FVector(MeteorInfo.Size));

	//운석의 속도에 따라 우주선 방향과 속도가 더해진 벡터로 운석의 이동 방향이 정해짐
	Direction = VSpaceShip - GetActorLocation() + (Velocity * Direction.Size());

	Direction.Normalize();

	Direction *= MeteorInfo.Speed;
}

