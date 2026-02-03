#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class ABullet : public AActor
{
	GENERATED_BODY()

public:
	ABullet();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Collision;

	UPROPERTY(VisibleAnywhere)
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	float Damage = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bullet")
	UParticleSystem* HitParticle;

	UPROPERTY()
	AActor* _owner;

	UFUNCTION()
	void OnOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHitEffect(const FVector& Location, const FRotator& Rotation);

public:
	// 발사 직후 방향 세팅
	void Fire(const FVector& Direction);

	void SetOwnerActor(AActor* InOwner)
	{
		_owner = InOwner;
	}

	void SetDamage(float damage)
	{
		Damage = damage;
	}

};