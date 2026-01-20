// Fill out your copyright notice in the Description page of Project Settings.

#include "PSJ_ShipCockpit.h"
#include "PSJ_Character.h" 
#include "PSJ_Spaceship.h"
#include "GameFramework/PlayerController.h"

void APSJ_ShipCockpit::OnInteractEnter()
{
    // 1. [팀원 코드] 부모 로직 실행 (UI 띄우기 등)
    Super::OnInteractEnter();

    // 2. 우주선 연결 확인
    if (TargetSpaceship)
    {
        // 현재 이 의자와 상호작용한 플레이어를 찾습니다.
        // (InteractableActorBase에 상호작용 주체(Instigator)를 주는 기능이 없다면,
        //  일반적으로 PlayerController 0번을 가져와 처리합니다.)
        APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
        
        if (APSJ_Character* MyChar = Cast<APSJ_Character>(PlayerPawn))
        {
            UE_LOG(LogTemp, Log, TEXT("Cockpit: Requesting Boarding..."));

            // 3. 우주선에게 "이 캐릭터 태워라" 명령
            TargetSpaceship->SetPilot(MyChar);
            
            // 4. 나중에 내릴 때 UI를 끄기 위해, 우주선에게 "내가 너의 조종석이야"라고 알려줌
            TargetSpaceship->LinkedCockpit = this;

            // 5. 컨트롤러 제어권 이양 (캐릭터 -> 우주선)
            if (APlayerController* PC = Cast<APlayerController>(MyChar->GetController()))
            {
                PC->Possess(TargetSpaceship);
            }
        }
    }
}

void APSJ_ShipCockpit::OnInteractExit()
{
    // 1. [팀원 코드] 부모 로직 실행 (UI 숨기기 등)
    Super::OnInteractExit();
    
    UE_LOG(LogTemp, Log, TEXT("Cockpit: Interact Exit (UI Reset)"));
}