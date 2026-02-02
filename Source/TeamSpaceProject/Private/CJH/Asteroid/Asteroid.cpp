#include "CJH/Asteroid/Asteroid.h"
#include "CJH/Asteroid/AsteroidComponent.h"

#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Player/SpaceStation.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"


// Sets default values

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	SpaceObjectComp = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;

	Collision->InitSphereRadius(5.f);

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);

	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	// ⭐ 핵심
	Collision->SetNotifyRigidBodyCollision(true);
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	SetAsteroidRot();

	HealthComp->OnDeath.AddDynamic(this, &AAsteroid::OnDestroy);
	Collision->OnComponentHit.AddDynamic(this, &AAsteroid::OnHit);
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
		return;
	
	MoveAsteroid(DeltaTime);

	float DestroyDist = FVector::Dist(SpaceStation->GetActorLocation(), GetActorLocation());
	
	if (DestroyDist > SpaceManager->GetSpaceRadius() - 1)
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

	SpaceManager->RemoveSpaceObject(SpaceObjectComp);
	 
	Super::EndPlay(EndPlayReason);
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

void AAsteroid::OnHit(
	UPrimitiveComponent* HitComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComponent, 
	FVector NormalImpulse, 
	const FHitResult& Hit)
{
	if (!OtherActor)
		return;

	if (!HitParticle)
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticle,
			Hit.ImpactPoint,
			Hit.ImpactNormal.Rotation()
		);
	/*UE_LOG(LogTemp, Warning, TEXT("%s"), *OtherActor->GetName())*/

	
	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
		Health->TakeDamage(_targetInfo.Attack_Damage);

	Destroy(); // 맞으면 사라짐
}

TObjectPtr<USpaceManager> AAsteroid::GetSpaceManager()
{
	if (SpaceManager = nullptr)
	{
		USpaceManager* _outSpaceManager = nullptr;
		if (!UStaticFunctionLibrary::TryGetSpaceManager(_outSpaceManager))
			return nullptr;

		SpaceManager = _outSpaceManager;
		SpaceStation = SpaceManager->GetSpaceStation();
	}

	return SpaceManager;
}