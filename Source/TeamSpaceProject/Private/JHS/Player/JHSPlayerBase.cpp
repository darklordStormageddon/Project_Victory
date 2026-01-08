// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Player/JHSPlayerBase.h"
#include "JHS/UI/UIInteracter.h"

#include "Kismet/GameplayStatics.h"
#include "JHS/SpaceObject/DriveSeatRader.h"

// Sets default values
AJHSPlayerBase::AJHSPlayerBase()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJHSPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	// ADriveSeatRader를 칮고 SetSpaceShip 함수 호출
	TArray<AActor*> _driveSeatRaderArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADriveSeatRader::StaticClass(), _driveSeatRaderArray);
	for (AActor* _driveSeatRader : _driveSeatRaderArray)
	{
		ADriveSeatRader* _driveSeatRaderActor = Cast<ADriveSeatRader>(_driveSeatRader);
		_driveSeatRaderActor->SetSpaceShip(this);
	}
}

// Called every frame
void AJHSPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AJHSPlayerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//void AJHSPlayerBase::InteractInput()
//{
//	if (_uiInteracter == nullptr)
//		return;
//
//	if (!_isInteract)
//	{
//		//_uiInteracter->OpenUI();
//		_isInteract = true;
//	}
//	else
//	{
//		//_uiInteracter->CloseUI();
//		_uiInteracter = nullptr;
//		_isInteract = false;
//	}
//}
//
//void AJHSPlayerBase::ChangeInteractable(TObjectPtr<UUIInteracter> UIInteracter)
//{
//	_uiInteracter = UIInteracter;
//}