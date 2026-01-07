// Fill out your copyright notice in the Description page of Project Settings.


#include "CJH/Stability/AS_Connection.h"

// Sets default values
AAS_Connection::AAS_Connection()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WingMesh"));
	WingMesh->SetupAttachment(RootComponent);

	OutlineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMesh"));
	OutlineMesh->SetupAttachment(WingMesh);

	OutlineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMesh->SetHiddenInGame(true);
	OutlineMesh->SetCastShadow(false);

	// 살짝 크게
	OutlineMesh->SetRelativeScale3D(FVector(1.05f));
}

// Called when the game starts or when spawned
void AAS_Connection::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAS_Connection::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(DelayAsk)
	{
		DelayAsk = false;

		GetWorld()->GetTimerManager().SetTimer(AskTimerHandle, this, &AAS_Connection::CanAsk, AskDelayTime, false);

		HaveChild();

		F_CanSeparate();
	}
}

void AAS_Connection::CanAsk()
{
	DelayAsk = true;
}

void AAS_Connection::F_CanSeparate()
{
	if (CanSeparate)
		OutlineMesh->SetHiddenInGame(false);
	else
		OutlineMesh->SetHiddenInGame(true);
}