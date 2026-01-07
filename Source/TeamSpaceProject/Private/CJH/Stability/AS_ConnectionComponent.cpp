// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/AS_ConnectionComponent.h"

// Sets default values
UAS_ConnectionComponent::UAS_ConnectionComponent()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

	ConnectionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ConnectionMesh"));
	ConnectionMesh->SetupAttachment(this);

	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->SetupAttachment(ConnectionMesh);

	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->SetHiddenInGame(true);
	OutlineMesh->SetCastShadow(false);

	// 살짝 크게
	OutlineMesh->SetRelativeScale3D(FVector(1.05f));
}

// Called when the game starts or when spawned
void UAS_ConnectionComponent::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void UAS_ConnectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DelayAsk)
	{
		DelayAsk = false;

		GetWorld()->GetTimerManager().SetTimer(AskTimerHandle, this, &UAS_ConnectionComponent::CanAsk, AskDelayTime, false);

		HaveChild();

		F_CanSeparate();
	}
}

void UAS_ConnectionComponent::HaveChild()
{
	CanSeparate = true; // 기본은 분리 가능

	// 1️⃣ Connection(this)의 자식들
	TArray<USceneComponent*> Children;
	GetChildrenComponents(false, Children); // false = 직계만

	UE_LOG(LogTemp,Warning, TEXT("My Name: %s"), *GetName());

	for (USceneComponent* Child : Children)
	{
		if (!Child)
			continue;
		if (Child == ConnectionMesh)
			continue;
		if (Child == OutlineMesh )
			continue;
		
		// 2️⃣ Wing 컴포넌트 찾기
		TArray<USceneComponent*> ChildComponents;
		Child->GetChildrenComponents(false, ChildComponents);
		//WingMesh를 제외한 자식 컴포넌트가 하나라도 있으면 분리 불가
		for (USceneComponent* GrandChild : ChildComponents)
		{
			if (!GrandChild)
				continue;
			if (GrandChild->GetName().Contains("WingMesh"))
				continue;

			CanSeparate = false;
		}
	}

}

void UAS_ConnectionComponent::CanAsk()
{
	DelayAsk = true;
}

void UAS_ConnectionComponent::F_CanSeparate()
{
	if (CanSeparate)
		OutlineMesh->SetHiddenInGame(false);
	else
		OutlineMesh->SetHiddenInGame(true);
}