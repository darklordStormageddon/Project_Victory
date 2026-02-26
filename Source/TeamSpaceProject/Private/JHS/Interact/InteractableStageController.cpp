// Fill out your copyright notice in the Description page of Project Settings.


#include "JHS/Interact/InteractableStageController.h"
#include "JHS/GameControl/StaticFunctionLibrary.h"
#include "JHS/GameControl/JHSGameMode.h"
#include "JHS/GameControl/JHSPlayerController.h"
#include "PSJ/TaskPawnBase.h"
#include "Kismet/GameplayStatics.h"


void AInteractableStageController::OnInteractEnter(int32 CallerPlayerId, TObjectPtr<UUIBase> OpenedUI)
{
	InteractController(CallerPlayerId);
}

void AInteractableStageController::OnInteractExit(int32 CallerPlayerId, TObjectPtr<UUIBase> ClosedUI)
{
}

void AInteractableStageController::InteractController(int32 CallerPlayerId)
{
	AJHSGameMode* _outGameMode = nullptr;
	if (!UStaticFunctionLibrary::TryGetGameMode(_outGameMode))
		return;

	AJHSPlayerController* _callerController = nullptr;
	if (!UStaticFunctionLibrary::TryGetPlayerController(_callerController))
		return;

	if (_callerController->GetAssignedPlayerId() != CallerPlayerId)
		return;

	if (_isStartStage)
	{
		_outGameMode->StartNextStage(_callerController);
	}
	else
	{
		// [추가된 로직] 로비로 이동(EndStage)하기 직전에 모든 탑승자를 강제 하차시킵니다.
		TArray<AActor*> FoundPawns;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATaskPawnBase::StaticClass(), FoundPawns);

		for (AActor* Actor : FoundPawns)
		{
			if (ATaskPawnBase* TaskPawn = Cast<ATaskPawnBase>(Actor))
			{
				// 해당 조종석에 누군가 탑승 중이라면 강제 하차 실행
				if (TaskPawn->CurrentPilot != nullptr)
				{
					TaskPawn->DisembarkCharacter();
				}
			}
		}

		// 탑승자 처리가 모두 끝난 후 안전하게 스테이지 종료 및 텔레포트 이벤트 진행
		_outGameMode->EndStage(_callerController);
	}
}