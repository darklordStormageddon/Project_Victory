// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/UI/UIInteracter.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/UI/UIManager.h"
#include "JHS/Player/JHSPlayerBase.h"
#include "Components/SceneComponent.h"

// Sets default values for this component's properties
UUIInteracter::UUIInteracter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UUIInteracter::BeginPlay()
{
	Super::BeginPlay();

	UUIManager* _outUIManager = nullptr;
	if (!UStaticFunctionLibrary::TryGetUIManager(_outUIManager))
		return;

	_uiManager = _outUIManager;

	// Collision 컴포넌트 생성 및 설정
	AActor* _owner = GetOwner();
	if (_owner != nullptr)
	{
		_collisionComponent = NewObject<UBoxComponent>(_owner, UBoxComponent::StaticClass(), TEXT("CollisionComponent"));
		if (_collisionComponent != nullptr)
		{
			_collisionComponent->SetBoxExtent(_collisionBoxExtent);
			_collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			_collisionComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
			_collisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			_collisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap);
			_collisionComponent->SetGenerateOverlapEvents(true);

			// Root Component에 Attach
			USceneComponent* _rootComponent = _owner->GetRootComponent();
			if (_rootComponent != nullptr)
			{
				_collisionComponent->SetupAttachment(_rootComponent);
			}

			// 컴포넌트를 액터에 추가
			_owner->AddInstanceComponent(_collisionComponent);
			_collisionComponent->RegisterComponent();

			// 델리게이트 바인딩
			_collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &UUIInteracter::OnTriggerEnter);
			_collisionComponent->OnComponentEndOverlap.AddDynamic(this, &UUIInteracter::OnTriggerExit);
		}
	}
}


// Called every frame
void UUIInteracter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UUIInteracter::OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr)
		return;

	AJHSPlayerBase* _player = Cast<AJHSPlayerBase>(OtherActor);
	if (_player != nullptr)
	{
		_player->ChangeInteractable(this);
		_uiManager->OpenUI(E_UI_TYPE::UIPanelPlayer);
	}
}

void UUIInteracter::OnTriggerExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr)
		return;

	AJHSPlayerBase* _player = Cast<AJHSPlayerBase>(OtherActor);
	if (_player != nullptr)
	{
		_player->ChangeInteractable(nullptr);
		_uiManager->CloseUI(E_UI_TYPE::UIPanelPlayer);
	}
}

void UUIInteracter::OpenUI()
{
	_uiManager->OpenUI(_openUIType);
	_uiManager->CloseUI(E_UI_TYPE::UIPanelPlayer);
}

void UUIInteracter::CloseUI()
{
	_uiManager->CloseUI(_openUIType);
}

