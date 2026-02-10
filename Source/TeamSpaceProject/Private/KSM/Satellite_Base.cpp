// Fill out your copyright notice in the Description page of Project Settings.

#include "KSM/Satellite_Base.h"
#include "KSM/PanelBase.h"
#include "KSM/BodyBase.h"
#include "KSM/Attachment_Base.h"
#include "KSM/Connector_Base.h"

#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/SpaceManager.h"

#include "JHS/SpaceObject/SpaceObjectComponent.h"

// Sets default values
ASatellite_Base::ASatellite_Base()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    // 루트
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // 자식 8개 생성
    SceneChild1 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild1"));
    SceneChild1->SetupAttachment(SceneRoot);

    SceneChild2 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild2"));
    SceneChild2->SetupAttachment(SceneRoot);

    SceneChild3 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild3"));
    SceneChild3->SetupAttachment(SceneRoot);

    SceneChild4 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild4"));
    SceneChild4->SetupAttachment(SceneRoot);

    SceneChild5 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild5"));
    SceneChild5->SetupAttachment(SceneRoot);

    SceneChild6 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild6"));
    SceneChild6->SetupAttachment(SceneRoot);

    SceneChild7 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild7"));
    SceneChild7->SetupAttachment(SceneRoot);

    SceneChild8 = CreateDefaultSubobject<USceneComponent>(TEXT("SceneChild8"));
    SceneChild8->SetupAttachment(SceneRoot);

    SpaceObjectComp = CreateDefaultSubobject<USpaceObjectComponent>(TEXT("SpaceObjectComponent"));

    bReplicates = true;
    bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ASatellite_Base::BeginPlay()
{
	Super::BeginPlay();
	
    GetSpaceManager();
}

// Called every frame
void ASatellite_Base::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASatellite_Base::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool ASatellite_Base::GetSpaceManager()
{
    if (SpaceManager == nullptr)  // Fixed: Use == for comparison instead of = for assignment
    {
        USpaceManager* _outSpaceManager = nullptr;
        if (!UStaticFunctionLibrary::TryGetSpaceManager(_outSpaceManager))
            return false;  // Changed: Return false if failed to get SpaceManager

        SpaceManager = _outSpaceManager;

        return true;  // Changed: Return true if successful
    }

    return true;
}

void ASatellite_Base::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(SpaceManager))
    {
        SpaceManager->RemoveSpaceObject(SpaceObjectComp);
    }

    Super::EndPlay(EndPlayReason);
}

