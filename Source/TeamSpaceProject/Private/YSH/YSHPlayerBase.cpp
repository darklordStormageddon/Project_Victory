#include "YSH/YSHPlayerBase.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "JHS/UI/UIInteracterable.h"


AYSHPlayerBase::AYSHPlayerBase()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AYSHPlayerBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AYSHPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AYSHPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
