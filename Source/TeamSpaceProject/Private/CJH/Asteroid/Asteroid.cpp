#include "CJH/Asteroid/Asteroid.h"
#include "CJH/Asteroid/AsteroidComponent.h"

#include "JHS/SpaceObject/SpaceRader.h"
#include "JHS/GameControl/SpaceManager.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/Player/SpaceStation.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"

#include "Net/UnrealNetwork.h"

// Sets default values

AAsteroid::AAsteroid()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(false);
	NetUpdateFrequency = 20.0f;
	MinNetUpdateFrequency = 10.0f;
	NetPriority = 1.0f;
	bAlwaysRelevant = false;
	SetNetDormancy(DORM_Awake);
	NetCullDistanceSquared = 10000000000.0f; // 100000 UU radius

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

	// Ensure SpaceManager is initialized before any RPC operations
	if (GetSpaceManager())
	{
		SetAsteroidRot();
	}

	HealthComp->OnDeath.AddDynamic(this, &AAsteroid::OnDestroy);
	Collision->OnComponentHit.AddDynamic(this, &AAsteroid::OnHit);
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MoveAsteroid(DeltaTime);

	if (!HasAuthority())
		return;

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
	Direction = (TargetLocation - GetActorLocation()).GetSafeNormal();
	Direction *= _targetInfo.Speed;

	AJHSGameMode* GameMode = Cast<AJHSGameMode>(GetWorld()->GetAuthGameMode());

	HealthComp->SetCurrentHP(_targetInfo.Max_HP);

	// ===== RPC: 모든 클라이언트에 정보 동기화 =====
	if (HasAuthority())
		MulticastSetAsteroidInfo(InAsteroidInfo, TargetLocation, Direction);
}

// ===== RPC 함수 추가 =====
void AAsteroid::MulticastSetAsteroidInfo_Implementation(
	const FTargetInfo& InAsteroidInfo,
	const FVector& InTargetLocation,
	const FVector& InDirection)
{
	_targetInfo = InAsteroidInfo;
	TargetLocation = InTargetLocation;
	Direction = InDirection;
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

	if (HasAuthority() && HitParticle)
		MulticastHitEffect(Hit.ImpactPoint, Hit.ImpactNormal.Rotation());

	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
		Health->TakeDamage(_targetInfo.Attack_Damage);

	Destroy(); // 맞으면 사라짐
}

void AAsteroid::MulticastHitEffect_Implementation(
	const FVector& ImpactPoint,
	const FRotator& ImpactRotation)
{
	if (GetNetMode() == NM_DedicatedServer)
		return;

	if (!HitParticle)
		return;

	const FTransform SpawnTransform(ImpactRotation, ImpactPoint, HitParticleScale);

	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitParticle,
		SpawnTransform
	);
}

bool AAsteroid::GetSpaceManager()
{
	if (SpaceManager == nullptr)  // Fixed: Use == for comparison instead of = for assignment
	{
		USpaceManager* _outSpaceManager = nullptr;
		if (!UStaticFunctionLibrary::TryGetSpaceManager(_outSpaceManager))
			return false;  // Changed: Return false if failed to get SpaceManager

		SpaceManager = _outSpaceManager;
		SpaceStation = SpaceManager->GetSpaceStation();
		
		return true;  // Changed: Return true if successful
	}

	return true;
}

void AAsteroid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAsteroid, Direction);
}