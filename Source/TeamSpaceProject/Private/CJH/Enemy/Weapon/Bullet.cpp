#include "CJH/Enemy/Weapon/Bullet.h"

#include "KSM/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABullet::ABullet()
{
	PrimaryActorTick.bCanEverTick = false; // ❌ Tick 필요 없음

	// ======================
	// Replication
	// ======================
	bReplicates = true;
	SetReplicateMovement(true);

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

	// Overlap 기반이므로 Hit 아님
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
	ProjectileMovement->SetIsReplicated(true);

	// 수명
	InitialLifeSpan = 3.0f;
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
	if (!HasAuthority())
		return;

	if (!OtherActor || OtherActor == _owner)
		return;

	MulticastHitEffect(
		SweepResult.ImpactPoint,
		SweepResult.ImpactNormal.Rotation()
	);

	if (UHealthComponent* Health = OtherActor->FindComponentByClass<UHealthComponent>())
		Health->TakeDamage(Damage);

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