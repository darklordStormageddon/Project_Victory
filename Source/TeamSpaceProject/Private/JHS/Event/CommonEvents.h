// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JHS/Event/CommonEventBase.h"
#include "CommonEvents.generated.h"

/**
 * Upgrade Attack Level 이벤트들
 */

// 투사체 추가 이벤트
UCLASS(BlueprintType)
class UEventOnAddProjectile : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnAddProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}
};

// 쿨타임 감소 이벤트
UCLASS(BlueprintType)
class UEventOnReduceCoolTime : public UCommonEventBase
{
	GENERATED_BODY()

public:
	UEventOnReduceCoolTime(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get())
		: Super(ObjectInitializer)
	{}
};
