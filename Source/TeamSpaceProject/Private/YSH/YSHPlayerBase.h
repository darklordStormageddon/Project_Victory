#pragma once

#include "CoreMinimal.h"
#include "TeamSpaceProject/TeamSpaceProjectCharacter.h"
#include "YSHPlayerBase.generated.h"

class AActor;
class ATurretBase;
class UUIInteracterable;

UCLASS()
class AYSHPlayerBase : public ATeamSpaceProjectCharacter
{
	GENERATED_BODY()

public:
	AYSHPlayerBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};