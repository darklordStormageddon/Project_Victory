#include "CJH/Enemy/Weapon/Bullet.h"

#include "KSM/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = false;

	// ======================
	// Replication
	// ======================
	bReplicates = true;  // ===== 클라이언트에서도 볼 수 있도록 활성화 =====
	SetReplicateMovement(true);  // 위치 리플리케이션 활성화

	// ======================
	// Collision
	// ======================
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;

	Collision->InitSphereRadius(5.f);

	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

	Collision->SetNotifyRigidBodyCollision(false);

	// ======================
	// Projectile Movement
	// ======================
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 4000.f;
	ProjectileMovement->MaxSpeed = 4000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->SetIsReplicated(true);  // 리플리케이션 활성화

	// ===== 최적화 =====
	InitialLifeSpan = 2.0f;
	NetUpdateFrequency = 60.0f;      // 클라에 주기적으로 위치 동기화
	MinNetUpdateFrequency = 30.0f;
	// 클라이언트에 스폰 패킷이 전달되기 전에 Destroy되지 않도록 최소 생존 시간 보장
	NetDormancy = DORM_Never;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ABullet::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ABullet::OnOverlap);
}

void ABullet::OnOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// 서버에서만 처리
	if (!HasAuthority())
		return;

	// ===== 유효성 검사 =====
	if (!OtherActor)
		return;

	// ===== Owner가 아직 살아있으면 자신 타격 방지 =====
	if (IsValid(_owner) && OtherActor == _owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Bullet: Hit own owner, ignoring"));
		return;
	}

	// ===== Owner의 Owner와도 비교 (Pawn이 Vehicle에 탑승한 경우) =====
	if (IsValid(_owner))
	{
		APawn* OwnerPawn = Cast<APawn>(_owner);
		if (OwnerPawn && OtherActor == OwnerPawn->GetOwner())
			return;
	}

	// ===== 이펙트 위치 검증 =====
	FVector ImpactLocation = SweepResult.ImpactPoint;
	
	// ImpactPoint가 0,0,0이면 현재 위치 사용
	if (ImpactLocation.IsNearlyZero())
	{
		ImpactLocation = GetActorLocation();
	}

	FRotator ImpactRotation = SweepResult.ImpactNormal.Rotation();

	// 이펙트 발동 (유효한 위치에서만)
	MulticastHitEffect(ImpactLocation, ImpactRotation);

	// 데미지 처리
	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
		Health->TakeDamage(Damage);

	// 즉시 제거
	Destroy();
}

void ABullet::Fire(const FVector& Direction)
{
	if (ProjectileMovement)
		ProjectileMovement->Velocity = Direction * ProjectileMovement->InitialSpeed;
}

void ABullet::MulticastHitEffect_Implementation(
	const FVector& Location,
	const FRotator& Rotation
)
{
	// 클라이언트에서만 실행 (이펙트는 시각적 용도)
	if (GetNetMode() == NM_DedicatedServer)
		return;

	if (HitParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticle,
			Location,
			Rotation
		);
	}
}