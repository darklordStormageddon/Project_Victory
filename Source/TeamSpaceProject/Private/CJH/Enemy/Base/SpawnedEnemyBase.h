// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CJH/Enemy/Base/EnemyBase.h"
#include "SpawnedEnemyBase.generated.h"

/**
 * 
 */
UCLASS()
class ASpawnedEnemyBase : public AEnemyBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	virtual void SetInfo() override;
};
